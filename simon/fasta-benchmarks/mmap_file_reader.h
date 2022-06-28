#pragma once

#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <array>
#include <filesystem>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

struct mmap_file_reader {
    int fd{-1};
    size_t filesize;
    char const* buffer;
    char const* endPtr;

    mmap_file_reader(char const* fname)
        : fd{[&]() {
            auto r = ::open(fname, O_RDONLY);
            if (r == -1) {
                throw "file not readable";
            }
            return r;
        }()}
        , filesize{file_size(std::filesystem::path{fname})}
        , buffer{[&]() {
            auto ptr = (char const*)mmap(nullptr, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
            return ptr;
        }()}
        , endPtr{buffer+filesize}
    {}

    mmap_file_reader() = delete;
    mmap_file_reader(mmap_file_reader const&) = delete;
    mmap_file_reader(mmap_file_reader&& _other) noexcept
        : fd{_other.fd}
        , filesize{_other.filesize}
        , buffer{_other.buffer}
        , endPtr{_other.endPtr}
    {
        _other.fd = -1;
    }
    auto operator=(mmap_file_reader const&) -> mmap_file_reader& = delete;
    auto operator=(mmap_file_reader&&) -> mmap_file_reader& = delete;

    ~mmap_file_reader() {
        if (fd == -1) return;

        munmap((void*)buffer, filesize);
        close(fd);
    }

    void doneUntil(char const* ptr) {
        auto i = ptr - buffer;
        auto mask = std::numeric_limits<size_t>::max() - 4095;
        munmap((void*)buffer, i & mask);
        buffer = buffer + (i & mask);
        filesize -= (i & mask);
    }
    auto size() const {
        return filesize;
    }

    friend auto begin(mmap_file_reader const& reader) {
        return reader.buffer;
    }
    friend auto end(mmap_file_reader const& reader) {
        return reader.endPtr;
    }
};
