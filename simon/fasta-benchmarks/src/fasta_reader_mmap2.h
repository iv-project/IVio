#pragma once

#include "fasta_reader_view.h"


struct mmap_queue {
    // this mmap_queue only sufficient for 1 Exbytes of data
    size_t filesize = 1'024ul*1'024ul*1'024ul*1'024ul*1'024ul*1'024ul;
    char* buffer {[&]() {
        auto ptr = (char*)mmap(nullptr, filesize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE, -1, 0);
        return ptr;
    }()};

    mmap_queue() = default;
    mmap_queue(mmap_queue&& _other) noexcept
        : filesize{_other.filesize}
        , buffer  {_other.buffer}
    {
        _other.buffer = nullptr;
    }

    ~mmap_queue() {
        if (buffer == nullptr) return;
        munmap((void*)buffer, filesize);
    }


    void dropUntil(char const* ptr) {
        auto i = ptr - buffer;
        auto mask = std::numeric_limits<size_t>::max() - 4095;
        munmap((void*)buffer, i & mask);
        buffer = buffer + (i & mask);
        filesize -= (i & mask);
    }

};


template <typename Reader>
struct fasta_reader_mmap2 {
    Reader file;

    size_t lastUsed{};

    mmap_queue buf;
    char* startPtr = buf.buffer;
    size_t size{};

    using record_view = fasta_reader_view_record_view;
    using iter        = fasta_reader_view_iter;


    bool valid() const {
        return (size_t)buf.buffer != -1;
    }

    friend auto begin(fasta_reader_mmap2& reader) {
        return iter{[&reader]() { return reader.next(); }};
    }
    friend auto end(fasta_reader_mmap2 const&) {
        return nullptr;
    }
    auto readMore() {
        uint64_t extra = 4'096*1'024;
        if (size/8 > extra) {
            extra = size/8;
        }
        auto bytes_read = file.read(std::ranges::subrange{startPtr + size, startPtr+size + extra});
        size += bytes_read;

        return bytes_read != 0;
    }
    size_t readUntil(char c) {
        while (true) {
            auto ptr = (char const*)memchr(startPtr + lastUsed, c, size - lastUsed);
            if (ptr != nullptr) {
                lastUsed = ptr - startPtr;
                return lastUsed;
            }
            if (!readMore()) {
                return size;
            }
        }
    }

    auto next() -> std::optional<record_view> {
        auto startId = readUntil('>');
        if (startId == size) return std::nullopt;
        size -= startId+1;
        startPtr = startPtr+startId+1;
        buf.dropUntil(startPtr);
        lastUsed = 0;
        startId = 0;

        auto endId = readUntil('\n');
        if (endId == size) return std::nullopt;

        auto startSeq = endId+1;
        auto endSeq = readUntil('>');
        if (endSeq != size) {
            lastUsed -= 1;
        }

        return record_view{
            .id  = std::string_view{startPtr+startId, startPtr+endId},
            .seq = std::string_view{startPtr+startSeq, startPtr + endSeq} | seq_cleanuped_view};
    }
};

