#pragma once

#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <array>
#include <string>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


struct file_reader {
    int fd{-1};

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

    auto operator=(file_reader const&) -> file_reader& = delete;
    auto operator=(file_reader&&) -> file_reader& = delete;

    ~file_reader() {
        if (fd == -1) return;
        close(fd);
    }

    size_t read(std::ranges::sized_range auto&& range) const {
        auto bytes_read = ::read(fd, &*std::begin(range), std::ranges::size(range));
        if (bytes_read == -1) {
            auto s = std::string{"read failed "} + strerror(errno);
            throw s;
        }
        return bytes_read;
    }
};

