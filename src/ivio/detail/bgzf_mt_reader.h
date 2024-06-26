// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "bgzf_reader.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <list>

namespace ivio {

//!WORKAROUND llvm < 18 and MSVC < 19.28 do not provide jthread
// libstdc++ defines `__GLIBCXX__` -> Has jthread
// libstd++ defines `_LIBCPP_VERSION` -> jthread since 18 but not on macOS? (!TODO not sure why on macos it doesn't work)
// msvc defines `_MSC_VER` -> jthread since 19.28
// !TODO this should work, but emcc can't find std::jthread
#if (defined(__GLIBCXX__) || (_LIBCPP_VERSION >= 180000 && !__APPLE__) || _MSC_VER >= 1928) && !defined(__EMSCRIPTEN__)

namespace bgzf_mt {

template <typename Job>
struct job_queue {
    struct WrappedJob {
        Job job;
        std::mutex mutex;
        std::condition_variable cv;
        bool processing{};
        bool doneflag{};

        void reset() {
            auto g = std::unique_lock{mutex};
            processing = false;
            doneflag = false;
        }

        void done() {
            auto g = std::unique_lock{mutex};
            doneflag = true;
            cv.notify_one();
        };

        void await() {
            auto g = std::unique_lock{mutex};
//            if (doneflag) return;
            cv.wait(g, [this]() {
                return doneflag;
            });
        }
    };


    std::mutex mutex;
    std::list<std::unique_ptr<WrappedJob>> jobs;
    bool terminate{};
    std::condition_variable cv;

    job_queue() = default;
    job_queue(job_queue const&) = delete;
    job_queue(job_queue&& _other) {
        auto g = std::scoped_lock(mutex, _other.mutex);

        jobs = std::move(_other.jobs);
    }
    auto operator=(job_queue const&) -> job_queue& = delete;
    auto operator=(job_queue&& _other) -> job_queue& {
        auto g = std::scoped_lock(mutex, _other.mutex);

        jobs = std::move(_other.jobs);
        return *this;
    }

    void init_jobs(size_t nbr) {
        for (size_t i{0}; i < nbr; ++i) {
            jobs.emplace_back(std::make_unique<WrappedJob>());
        }
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
        return jobs.front().get();
    }

    void recycle() {
        auto g = std::unique_lock(mutex);
        auto ptr = std::move(jobs.front());
        jobs.pop_front();
        ptr->reset();
        jobs.emplace_back(std::move(ptr));
        cv.notify_one();
    }

    auto process_job() -> WrappedJob* {
        while (true) {
            auto g = std::unique_lock(mutex);
            for (auto& job : jobs) {
                auto g2 = std::unique_lock{job->mutex};
                if (job->processing == false) {
                    job->processing = true;
                    return job.get();
                }
            }
            if(terminate) return nullptr;
            cv.wait(g);
        }
    }
};
}

struct bgzf_mt_reader {
    struct Job {
        std::string                  decompressedOutput{std::string(1<<16, '\0')};
        std::string_view             output_view{};
        std::string                  compressedInput;
        std::unique_ptr<ZlibContext> zlibCtx{std::make_unique<ZlibContext>()};
    };

    std::mutex ureaderMutex;
    VarBufferedReader reader;

    bgzf_mt::job_queue<Job>  jobs;
    std::vector<std::jthread> threads;

    // returns if should be aborted
    bool work() {
        auto g = std::unique_lock{ureaderMutex};
        auto job = jobs.process_job();
        if (!job) return true;

        auto& input  = job->job.compressedInput;
        {
            // copy from underlying buffer (locked)
            // into the Job buffer
            auto [ptr, avail_in] = reader.read(18);
            if (avail_in == 0) return false; // End of processing
            if (avail_in < 18) throw std::runtime_error{"failed reading (1)"};

            size_t compressedLen = bgzfUnpack<uint16_t>(ptr + 16) + 1u;
            std::tie(ptr, avail_in) = reader.read(compressedLen);
            if (avail_in < compressedLen) throw std::runtime_error{"failed reading (2)"};
            input.resize(compressedLen-18);
            std::memcpy(input.data(), ptr+18, compressedLen-18);
            reader.dropUntil(compressedLen);
            g.unlock();
        }

        auto& output      = job->job.decompressedOutput;
        auto& zlibCtx     = job->job.zlibCtx;
        auto& output_view = job->job.output_view;

        size_t size = zlibCtx->decompressBlock({input.begin(), input.size()}, {output.data(), output.size()});
        output_view = {output.begin(), output.begin() + size};
        job->done();
        return false;
    }

    void startThread(size_t threadNbr) {
        while (threads.size() < threadNbr) {
            threads.emplace_back(std::jthread{[this](std::stop_token stoken) {
                while(!stoken.stop_requested()) {
                    if (work()) {
                        return;
                    }
                }
            }});
        }
    }

    bgzf_mt_reader(VarBufferedReader reader_, size_t threadNbr=1)
        : reader{std::move(reader_)}
    {
        startThread(threadNbr);
        jobs.init_jobs(threadNbr+1);
    }

    bgzf_mt_reader(bgzf_mt_reader const&) = delete;
    bgzf_mt_reader(bgzf_mt_reader&& _other) {
        auto threadNbr = _other.threads.size();
        _other.jobs.finish();
        _other.threads.clear();
        jobs   = std::move(_other.jobs);
        reader = std::move(_other.reader);
        startThread(threadNbr);
    }

    auto operator=(bgzf_mt_reader const&) -> bgzf_mt_reader& = delete;
    auto operator=(bgzf_mt_reader&&) -> bgzf_mt_reader& = delete;


    ~bgzf_mt_reader() {
        jobs.finish();
    }

    size_t read(std::ranges::contiguous_range auto&& range) {
        static_assert(std::same_as<std::ranges::range_value_t<decltype(range)>, char>);
        auto next = jobs.begin();
        if (!next) return 0; // abort, nothing to do, finished everything

        next->await();

        auto& output_view = next->job.output_view;

        size_t size = std::min(range.size(), output_view.size());
        std::memcpy(range.data(), output_view.data(), size);
        output_view = output_view.substr(size);

        if (output_view.empty()) {
            jobs.recycle();
        }

        return size;
    }
};

#else

struct bgzf_mt_reader : bgzf_reader {
    bgzf_mt_reader(VarBufferedReader reader_, size_t threadNbr=1)
        : bgzf_reader{std::move(reader_)}
    {
        (void)threadNbr;
    }
};


#endif
static_assert(Readable<bgzf_mt_reader>);

}
