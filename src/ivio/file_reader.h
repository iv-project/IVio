#pragma once

#include "concepts.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <ranges>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <tuple>
#include <unistd.h>

namespace ivio {

class file_reader {
protected:
    int fd;
    size_t filesize_;

public:
    file_reader(std::filesystem::path path)
        : fd{[&]() {
            auto r = ::open(path.c_str(), O_RDONLY);
            if (r == -1) {
                throw std::runtime_error("file not readable: " + path.string());
            }
            return r;
        }()}
        , filesize_{std::filesystem::file_size(path)}
    {}

    file_reader() = delete;
    file_reader(file_reader const&) = delete;
    file_reader(file_reader&& _other) noexcept
        : fd{_other.fd}
        , filesize_{_other.filesize_}
    {
        _other.fd = -1;
    }

    ~file_reader() {
        if (fd == -1) return;
        ::close(fd);
    }

    auto operator=(file_reader const&) -> file_reader& = delete;
    auto operator=(file_reader&&) -> file_reader& = delete;

    auto getFileHandler() {
        return fd;
    }

    size_t read(std::span<char> range) const {
        auto bytes_read = ::read(fd, range.data(), range.size());
        if (bytes_read == -1) {
            throw std::runtime_error{std::string{"read failed "} + strerror(errno)};
        }
        return bytes_read;
    }

    auto filesize() const -> size_t {
        return filesize_;
    }
};

static_assert(Readable<file_reader>);
}
