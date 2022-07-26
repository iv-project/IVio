#pragma once

#include "concepts.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <ranges>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <tuple>
#include <unistd.h>

namespace io3 {

template <typename Reader, size_t minV = 4096>
class buffered_reader : public Reader {
    std::vector<char> buf = []() { auto vec = std::vector<char>{}; vec.reserve(minV); return vec; }();

public:
    using Reader::Reader;
    buffered_reader(Reader&& _other)
        : Reader{std::move(_other)}
    {}
    buffered_reader(buffered_reader const&) = delete;
    buffered_reader(buffered_reader&& _other)
        : Reader{std::move(_other)}
        , buf{std::move(_other.buf)}
    {}

private:
    auto readMore() -> bool {
        size_t lastSize = buf.size();
        if (buf.capacity() - buf.size() >= minV) {
            buf.resize(buf.capacity());
        } else {
            buf.resize(buf.capacity()*2);
        }

        auto bytes_read = Reader::read(std::ranges::subrange{buf.data() + lastSize, &*buf.end()});
        buf.resize(lastSize + bytes_read);

        return bytes_read != 0;
    }

public:
    size_t readUntil(char c, size_t lastUsed) {
        while (true) {
            auto ptr = (char const*)memchr(buf.data() + lastUsed, c, buf.size() - lastUsed);
            if (ptr != nullptr) {
                lastUsed = ptr - buf.data();
                return lastUsed;
    }
            if (!readMore()) {
                return buf.size();
            }
        }
    }

    auto read(size_t ct) -> std::tuple<char const*, size_t> {
        while (buf.size() < ct) {
            if (!readMore()) break;
        }
        return {buf.data(), buf.size()};
    }

    size_t dropUntil(size_t i) {
        if (i < minV) return i;
        std::copy(begin(buf)+i, end(buf), begin(buf));
        buf.resize(buf.size()-i);
        return 0;
    }

    bool eof(size_t i) const {
        return i == buf.size();
    }

    auto string_view(size_t start, size_t end) -> std::string_view {
        return std::string_view{buf.data()+start, buf.data()+end};
    }
};

}
