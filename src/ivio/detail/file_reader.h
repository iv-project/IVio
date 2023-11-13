// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "concepts.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <ranges>
#include <string>
#include <tuple>

#if (defined(unix) || defined(__unix__) || defined(__unix))

#include <fcntl.h>
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
    file_reader(std::filesystem::path const& path)
        : fd{[&]() {
            auto r = ::open(path.c_str(), O_RDONLY);
            if (r == -1) {
                throw std::runtime_error{"file " + path.string() + " not readable"};
            }
            return r;
        }()}
        , filesize_{file_size(path)}
    {}

    file_reader() = delete;
    file_reader(file_reader const&) = delete;
    file_reader(file_reader&& _other) noexcept
        : fd{_other.fd}
        , filesize_{_other.filesize_}
    {
        _other.fd = -1;
        _other.filesize_ = 0;
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

    void seek(size_t offset) {
        ::lseek64(fd, offset, SEEK_SET);
    }

    auto tell() const -> size_t {
        return lseek64(fd, 0, SEEK_CUR);
    }
};

}
#else

#include "stream_reader.h"

#include <fstream>

namespace ivio {

class file_reader {
protected:
    std::ifstream ifs;
    stream_reader reader;
public:
    file_reader(std::filesystem::path const& path)
        : ifs{path, std::ios::binary}
        , reader{ifs}
    {}

    file_reader() = delete;
    file_reader(file_reader const&) = delete;
    file_reader(file_reader&& _other) noexcept = default;

    ~file_reader() = default;
    auto operator=(file_reader const&) -> file_reader& = delete;
    auto operator=(file_reader&&) -> file_reader& = delete;

    size_t read(std::span<char> range) const { return reader.read(range); }
    void seek(size_t offset) { reader.seek(offset); }
    auto tell() const -> size_t { return reader.tell(); }
};

}
#endif

static_assert(ivio::Readable<ivio::file_reader>);
static_assert(ivio::Seekable<ivio::file_reader>);
