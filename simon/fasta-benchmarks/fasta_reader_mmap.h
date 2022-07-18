#pragma once

#include "fasta_reader_view.h"

template <typename Reader>
struct fasta_reader_mmap {
    Reader file;

    char const* currentPtr{begin(file)};

    using record_view = fasta_reader_view_record_view;
    using iter = fasta_reader_view_iter;

    friend auto begin(fasta_reader_mmap& reader) {
        return iter{[&reader]() { return reader.next(); }};
    }
    friend auto end(fasta_reader_mmap const&) {
        return nullptr;
    }

    char const* readUntil(char c, char const* ptr) const {
        return (char const*)memchr(ptr, c, end(file) - currentPtr);
    }

    auto next() -> std::optional<record_view> {
        auto startPtr = readUntil('>', currentPtr);
        if (startPtr == nullptr) return std::nullopt;
        ++startPtr;
        file.doneUntil(startPtr);

        auto endPtr = readUntil('\n', startPtr);
        if (endPtr == nullptr) return std::nullopt;

        auto startSeqPtr = endPtr+1;
        auto endSeqPtr = readUntil('>', startSeqPtr);
        if (endSeqPtr == nullptr) {
            endSeqPtr = end(file);
        }
        currentPtr = endSeqPtr;

        return record_view{
            .id  = std::string_view{startPtr, endPtr},
            .seq = std::string_view{startSeqPtr, endSeqPtr} | seq_cleanuped_view};
    }
};
