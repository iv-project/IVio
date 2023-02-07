#pragma once

#include "bgzf_mt_reader.h"

#include <condition_variable>
#include <mutex>
#include <thread>

namespace io3 {

struct reader_mt_phase {
    std::atomic_int phase{0};

    std::mutex mutex;
    std::condition_variable cv0;
    std::condition_variable cv1;

    void waitFor(int expected) {
        auto g = std::unique_lock{mutex};
        if (phase == expected) return;
        if (expected == 0) {
            cv0.wait(g);
        } else if (expected == 1) {
            cv1.wait(g);
        }
        phase = -1;
    }

    void trigger(int _phase) {
        auto g = std::unique_lock{mutex};
        phase = _phase;
        g.unlock();
        if (phase == 0) {
            cv0.notify_one();
        } else if (phase == 1) {
            cv1.notify_one();
        }
    }
};

template <size_t minV = 1<<12>
struct bgzf_mt_reader {
    VarBufferedReader reader;
    ZlibContext       zlibCtx;

    reader_mt_phase phase;
    std::jthread thread;
    std::string_view next;
    std::mutex mutex;
    bool threadStarted{};

    void startThread() {
        if (threadStarted) return;
        threadStarted = true;
        thread = std::jthread{[this](std::stop_token stoken) {
            size_t lastUsed{};
            struct R {
                size_t lastUsed{};
                std::string range;
            };
            auto   s = std::array<R, 3>{};
            size_t currentS{};

            while(!stoken.stop_requested()) {
                auto& s0          = s[currentS];

                reader.dropUntil(s0.lastUsed);

                s[(currentS+1)%3].lastUsed -= s0.lastUsed;
                s[(currentS+2)%3].lastUsed -= s0.lastUsed;
                lastUsed -= s0.lastUsed;

                // the actual reading
                    auto [ptr, avail_in] = reader.read(lastUsed + 18);
                    if (avail_in == lastUsed) {
                        phase.waitFor(0);
                        next = {};
                        phase.trigger(1);
                        return;
                    }
                    if (avail_in < lastUsed + 18) throw "failed reading (1)";

                    size_t compressedLen = bgzfUnpack<uint16_t>(lastUsed + ptr + 16) + 1u;
                    auto [ptr2, avail_in2] = reader.read(lastUsed + compressedLen);
                    if (avail_in2 < lastUsed + compressedLen) throw "failed reading (2)";

                    s0.range.resize(1<<16);

                    assert(s0.range.size() >= (1<<16));

                    size_t size = zlibCtx.decompressBlock({lastUsed + ptr2+18, compressedLen-18}, {s0.range.data(), s0.range.size()});
                    s0.range.resize(size);
                    lastUsed = lastUsed + compressedLen;

                // end of reading
                phase.waitFor(0);
                next = s0.range;
                phase.trigger(1);
                currentS = (currentS + 1) % 3;
                s0.lastUsed = lastUsed;
            }
        }};

    }

    bgzf_mt_reader(VarBufferedReader reader_)
        : reader{std::move(reader_)}
    {}

    bgzf_mt_reader(bgzf_mt_reader const&) = delete;
    bgzf_mt_reader(bgzf_mt_reader&& _other)
        : reader{std::move(_other.reader)}
        , thread{std::move(_other.thread)}
    {
        assert(!_other.threadStarted);
    }

    auto operator=(bgzf_mt_reader const&) -> bgzf_mt_reader& = delete;
    auto operator=(bgzf_mt_reader&&) -> bgzf_mt_reader& = delete;


    ~bgzf_mt_reader() = default;

/*    size_t read(std::ranges::sized_range auto&& range) {
        startThread();
        phase.waitFor(1);
        assert(range.size() >= next.size());
        std::memcpy(range.data(), next.data(), next.size());
        auto size = next.size();
        phase.trigger(0);
        return size;
    }*/

    std::vector<char> buf = []() { auto vec = std::vector<char>{}; vec.reserve(minV); return vec; }();
    int inPos{};

    auto readMore() -> bool {
        startThread();
        phase.waitFor(1);
        if (next.size() == 0) {
            phase.trigger(0);
            return false;
        }

        auto oldSize = buf.size();
        buf.resize(buf.size() + next.size());
        std::copy_n(next.begin(), next.size(), buf.begin() + oldSize);
        phase.trigger(0);
        return true;
    }


    size_t readUntil(char c, size_t lastUsed) {
        while (true) {
            auto pos = std::string_view{buf.data(), buf.size()}.find(c, lastUsed + inPos);
            if (pos != std::string_view::npos) {
                return pos - inPos;
            }

            if (!readMore()) {
                return buf.size() - inPos;
            }
        }
    }

    auto read(size_t ct) -> std::tuple<char const*, size_t> {
        while (buf.size()-inPos < ct) {
            if (!readMore()) break;
        }
        return {buf.data()+inPos, buf.size()-inPos};
    }

    void dropUntil(size_t i) {
        i = i + inPos;
        if (i < minV) {
            inPos = i;
            return;
        }
        std::copy(begin(buf)+i, end(buf), begin(buf));
        buf.resize(buf.size()-i);
        inPos = 0;
    }

    bool eof(size_t i) const {
        return i+inPos == buf.size();
    }

    auto string_view(size_t start, size_t end) -> std::string_view {
        return std::string_view{buf.data()+start+inPos, buf.data()+end+inPos};
    }
};
//static_assert(Readable<bgzf_mt_reader>);
static_assert(BufferedReadable<bgzf_mt_reader<>>);

}
