#pragma once

#include "buffered_reader.h"
#include "fasta_reader_meta.h"

namespace io3 {

template <typename Reader>
struct fasta_reader_impl {
    Reader reader;
    size_t lastUsed{};
    std::string s;

    using record_view = fasta::record_view;
    using iter        = fasta::iter;

    fasta_reader_impl(Reader&& r)
        : reader{std::move(r)}
    {}

    fasta_reader_impl(fasta_reader_impl const&) = delete;
    fasta_reader_impl(fasta_reader_impl&& _other) = default;
    ~fasta_reader_impl() = default;

    friend auto begin(fasta_reader_impl& reader) {
        return iter{[&reader]() { return reader.next(); }};
    }
    friend auto end(fasta_reader_impl const&) {
        return nullptr;
    }

    auto next() -> std::optional<record_view> {
        auto startId = reader.readUntil('>', lastUsed);
        if (reader.eof(startId)) return std::nullopt;
        reader.dropUntil(startId+1);

        auto endId = reader.readUntil('\n', 0);
        if (reader.eof(endId)) return std::nullopt;

        auto startSeq = endId+1;
        auto endSeq = reader.readUntil('>', startSeq);
        lastUsed = endSeq;

        // convert into dense string representation
        s.clear();
        s.reserve(endSeq-startSeq);
        {
            auto s2 = startSeq;
            do {
                auto s1 = s2;
                s2 = reader.readUntil('\n', s1);
                s += reader.string_view(s1, s2);
                s2 += 1;
            } while(s2 < endSeq);
        }

        return record_view {
            .id  = reader.string_view(0,        endId),
            .seq = s,
        };
    }
};

static_assert(record_reader_c<fasta_reader_impl<io3::buffered_reader<io3::file_reader>>>);
}
