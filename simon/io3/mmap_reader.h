#pragma once

#include "file_reader.h"

#include <array>
#include <cassert>
#include <filesystem>

namespace io3 {

class mmap_reader : public file_reader {
protected:
    size_t filesize;
    char const* buffer;
    size_t inPos{};

public:
    mmap_reader(char const* fname)
        : file_reader{fname}
        , filesize{file_size(std::filesystem::path{fname})}
        , buffer{[&]() {
            auto ptr = (char const*)mmap(nullptr, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
            return ptr;
        }()}
    {
        assert(buffer);
    }

    mmap_reader() = delete;
    mmap_reader(mmap_reader const&) = delete;
    mmap_reader(mmap_reader&& _other) noexcept
        : file_reader{std::move(_other)}
        , filesize{_other.filesize}
        , buffer{_other.buffer}
        , inPos{_other.inPos}
    {
        assert(buffer);
        _other.buffer = nullptr;
    }
    auto operator=(mmap_reader const&) -> mmap_reader& = delete;
    auto operator=(mmap_reader&&) -> mmap_reader& = delete;

    ~mmap_reader() {
        if (buffer == nullptr) return;
        munmap((void*)buffer, filesize);
    }

    size_t readUntil(char c, size_t lastUsed) {
        lastUsed += inPos;
        assert(lastUsed <= filesize);
        auto pos = std::string_view{buffer, filesize}.find(c, lastUsed);
        if (pos != std::string_view::npos) {
            return pos - inPos;
        }
        return filesize - inPos;
    }

    auto read(size_t) -> std::tuple<char const*, size_t> {
        return {buffer+inPos, filesize-inPos};
    }

    void dropUntil(size_t i) {
        i += inPos;
        assert(i <= filesize);
        if (i < 1'024ul * 1'024ul) {
            inPos = i;
            return;
        }

        auto mask = std::numeric_limits<size_t>::max() - 4095;
        auto diff = (i & mask);
        assert(diff-inPos < filesize);
        munmap((void*)buffer, diff);
        buffer = buffer + diff;
        filesize -= diff;
        inPos = i - diff;
    }

    bool eof(size_t i) const {
        i += inPos;
        assert(i <= filesize);
        return filesize == i;
    }

    auto string_view(size_t start, size_t end) -> std::string_view {
        start += inPos;
        end   += inPos;
        assert(start <= filesize);
        assert(end <= filesize);
        return std::string_view{buffer+start, buffer+end};
    }

    auto size() const {
        return filesize;
    }
};

static_assert(BufferedReadable<mmap_reader>);
}
