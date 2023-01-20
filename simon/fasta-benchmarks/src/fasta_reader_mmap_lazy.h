#pragma once

#include "fasta_reader_view.h"

struct fasta_reader_lazy_record_view {
    using seq_view = decltype(std::string_view{} | seq_cleanuped_view);

    std::string_view id;
    seq_view seq;
};

struct fasta_reader_lazy_iter {
    using record_view = fasta_reader_lazy_record_view;
    std::function<std::optional<record_view>()> next;
    std::optional<record_view> nextItem = next();

    auto operator*() const -> record_view {
       return *nextItem;
    }
    auto operator++() -> fasta_reader_lazy_iter& {
        nextItem = next();
        return *this;
    }
    auto operator!=(std::nullptr_t _end) const {
        return nextItem.has_value();
    }
};

template <typename Reader>
struct fasta_reader_mmap_lazy {
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

    struct lazy_string_iter {
        fasta_reader_mmap_lazy& reader;

        auto operator*() const -> char {
            *reader.currentPtr;
        }
        auto operator++() -> lazy_string_iter {
            ++reader.currentPtr;
        }
        auto operator!=(std::nullptr_t) const {
            reader.currentPtr != end(reader.file);
        }
    };



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

        return record_view {
            .id  = std::string_view{startPtr, endPtr},
            .seq = std::string_view{startSeqPtr, endSeqPtr} | seq_cleanuped_view
        };
    }
};
