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
        ::close(fd);
    }

    auto operator=(file_reader const&) -> file_reader& = delete;
    auto operator=(file_reader&&) -> file_reader& = delete;

    size_t read(std::span<char> range) const {
        auto bytes_read = ::read(fd, range.data(), range.size());
        if (bytes_read == -1) {
            auto s = std::string{"read failed "} + strerror(errno);
            throw s;
        }
        return bytes_read;
    }
};

static_assert(reader_c<file_reader>);
}
