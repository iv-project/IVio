#pragma once

#include "concepts.h"

#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <array>
#include <string>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace io3 {


class file_reader {
protected:
    int fd{-1};

public:
    file_reader(char const* fname)
        : fd{[&]() {
            auto r = ::open(fname, O_RDONLY);
            if (r == -1) {
                throw "file not readable";
            }
            return r;
        }()}
    {}

    file_reader() = delete;
    file_reader(file_reader const&) = delete;
    file_reader(file_reader&& _other) noexcept
        : fd{_other.fd}
    {
        _other.fd = -1;
    }

    ~file_reader() {
        if (fd == -1) return;
        close(fd);
    }

    auto operator=(file_reader const&) -> file_reader& = delete;
    auto operator=(file_reader&&) -> file_reader& = delete;

    size_t read(std::ranges::sized_range auto&& range) const {
        auto bytes_read = ::read(fd, &*std::begin(range), std::ranges::size(range));
        if (bytes_read == -1) {
            auto s = std::string{"read failed "} + strerror(errno);
            throw s;
        }
        return bytes_read;
    }
};

static_assert(reader_c<file_reader>);

template <typename Reader>
class buffered_reader : public Reader {
    std::vector<char> buf = []() { auto vec = std::vector<char>{}; vec.reserve(4096); return vec; }();

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
        if (buf.capacity() - buf.size() >= 4*1024ul) {
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
        buf.erase(begin(buf), begin(buf)+i);
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
