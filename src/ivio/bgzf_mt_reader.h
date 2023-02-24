#pragma once

#include "bgzf_reader.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <list>

namespace ivio {

namespace bgzf_mt {
template <typename Job>
struct job_queue {
    struct WrappedJob {
        Job job;
        std::mutex mutex;
        std::condition_variable cv;
        bool processing{};
        bool doneflag{};

        void done() {
            auto g = std::unique_lock{mutex};
            doneflag = true;
            cv.notify_one();
        };
        void await() {
            auto g = std::unique_lock{mutex};
            if (doneflag) return;
            cv.wait(g);
        }
    };
    std::mutex mutex;
    std::list<WrappedJob> jobs;
    bool terminate{};
    std::condition_variable cv;

    job_queue() = default;
    job_queue(job_queue const&) = delete;
    job_queue(job_queue&& _other) {
        _other.finish();
        jobs = std::move(_other.jobs);
    }
    auto operator=(job_queue const&) -> job_queue& = delete;
    auto operator=(job_queue&& _other) -> job_queue& {
        _other.finish();
        jobs = std::move(_other.jobs);
        return *this;
    }

    ~job_queue() {
        finish();
    }

    void finish() {
        auto g = std::unique_lock(mutex);
        terminate = true;
        cv.notify_all();
    }

    auto begin() -> WrappedJob* {
        auto g = std::unique_lock(mutex);
        if (jobs.empty()) return nullptr;
        return &jobs.front();
    }

    void pop_front() {
        auto g = std::unique_lock(mutex);
        jobs.pop_front();
    }

    auto add_job(Job job) -> WrappedJob& {
        auto g = std::unique_lock(mutex);
        jobs.emplace_back(std::move(job));
        cv.notify_one();
        return jobs.back();
    }

    auto process_job() -> WrappedJob* {
        while (!terminate) {
            auto g = std::unique_lock(mutex);
            for (auto& job : jobs) {
                auto g2 = std::unique_lock{job.mutex};
                if (job.processing == false) {
                    job.processing = true;
                    return &job;
                }
            }
            if(terminate) break;
            cv.wait(g);
        }
        return nullptr;
    }
};
}

struct bgzf_mt_reader {
    struct Job {
        std::string                  range{std::string(1<<16, '\0')};
        std::string_view             range_view{range};
        std::string                  buffer;
        std::unique_ptr<ZlibContext> zlibCtx{std::make_unique<ZlibContext>()};
    };

    std::mutex ureaderMutex;
    VarBufferedReader reader;

    bgzf_mt::job_queue<Job>  jobs;
    std::vector<std::jthread> threads;

    void work(bgzf_mt::job_queue<Job>::WrappedJob* job) {
        auto& range   = job->job.range;
        auto& buffer  = job->job.buffer;
        auto& zlibCtx = job->job.zlibCtx;

        {
            auto g = std::unique_lock{ureaderMutex};
            auto [ptr, avail_in] = reader.read(18);
            if (avail_in == 0) {
                job->done();
                return;
            }
            if (avail_in < 18) throw "failed reading (1)";

            size_t compressedLen = bgzfUnpack<uint16_t>(ptr + 16) + 1u;
            std::tie(ptr, avail_in) = reader.read(compressedLen);
            if (avail_in < compressedLen) throw "failed reading (2)";
            buffer.resize(compressedLen-18);
            std::memcpy(buffer.data(), ptr+18, compressedLen-18);

            reader.dropUntil(compressedLen);
        }

        size_t size = zlibCtx->decompressBlock({buffer.begin(), buffer.size()}, {range.data(), range.size()});
        job->job.range_view = {range.begin(), range.begin() + size};

        // end of reading
        job->done();
    }

    void startThread(size_t threadNbr) {
        while (threads.size() < threadNbr) {
            threads.emplace_back(std::jthread{[this](std::stop_token stoken) {
                while(!stoken.stop_requested()) {
                    auto job = jobs.process_job();
                    if (!job) return;
                    work(job);
                }
            }});
        }
    }

    bgzf_mt_reader(VarBufferedReader reader_, size_t threadNbr=1)
        : reader{std::move(reader_)}
    {
        startThread(threadNbr);
        for (size_t i{0}; i < threadNbr+1; ++i) {
            jobs.add_job(Job{});
        }
    }

    bgzf_mt_reader(bgzf_mt_reader const&) = delete;
    bgzf_mt_reader(bgzf_mt_reader&& _other) {
        auto threadNbr = _other.threads.size();
        jobs   = std::move(_other.jobs);
        _other.threads.clear();
        reader = std::move(_other.reader);
        startThread(threadNbr);
    }

    auto operator=(bgzf_mt_reader const&) -> bgzf_mt_reader& = delete;
    auto operator=(bgzf_mt_reader&&) -> bgzf_mt_reader& = delete;


    ~bgzf_mt_reader() {
        jobs.finish();
    }

    size_t read(std::ranges::sized_range auto&& range) {
        auto next = jobs.begin();
        if (!next) return 0; // abort, nothing anymore todo
        next->await();
        size_t size = std::min(range.size(), next->job.range_view.size());
        std::memcpy(range.data(), next->job.range_view.data(), size);
        if (size == next->job.range_view.size()) {
            jobs.add_job(std::move(next->job));
            jobs.pop_front();
        } else {
            next->job.range_view = {next->job.range_view.data()+size, next->job.range_view.size() - size};
        }
        return size;
    }
};
static_assert(Readable<bgzf_mt_reader>);

}
