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
    size_t filesize;

public:
    file_reader(std::filesystem::path path)
        : fd{[&]() {
            auto r = ::open(path.c_str(), O_RDONLY); //!TODO Is this call safe?
            if (r == -1) {
                throw "file not readable";
            }
            return r;
        }()}
        , filesize{std::filesystem::file_size(path)}
    {}

    file_reader() = delete;
    file_reader(file_reader const&) = delete;
    file_reader(file_reader&& _other) noexcept
        : fd{_other.fd}
        , filesize{_other.filesize}
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

    auto file_size() const -> size_t {
        return filesize;
    }
};

static_assert(Readable<file_reader>);
}
