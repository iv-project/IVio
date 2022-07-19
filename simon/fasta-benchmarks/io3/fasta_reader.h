#pragma once

#include <functional>

namespace io3 {

struct fasta_reader_view_record_view {
    std::string_view id;
    std::string_view seq;
};

struct fasta_reader_view_iter {
    using record_view = fasta_reader_view_record_view;
    std::function<std::optional<record_view>()> next;
    std::optional<record_view> nextItem = next();

    auto operator*() const -> record_view {
       return *nextItem;
    }
    auto operator++() -> fasta_reader_view_iter& {
        nextItem = next();
        return *this;
    }
    auto operator!=(std::nullptr_t _end) const {
        return nextItem.has_value();
    }
};


template <typename Reader>
struct fasta_reader {
    Reader reader;

    using record_view = fasta_reader_view_record_view;
    using iter        = fasta_reader_view_iter;

    template <typename R>
    fasta_reader(R&& r)
        : reader{std::move(r)}
    {}

    fasta_reader(fasta_reader const&) = delete;
    fasta_reader(fasta_reader&& _other) noexcept = default;
    ~fasta_reader() = default;

    size_t lastUsed{};

    friend auto begin(fasta_reader& reader) {
        return iter{[&reader]() { return reader.next(); }};
    }
    friend auto end(fasta_reader const&) {
        return nullptr;
    }

    auto next() -> std::optional<record_view> {
        auto startId = reader.readUntil('>', lastUsed);
        if (reader.eof(startId)) return std::nullopt;
        startId = reader.dropUntil(startId+1);

        auto endId = reader.readUntil('\n', startId);
        if (reader.eof(endId)) return std::nullopt;

        auto startSeq = endId+1;
        auto endSeq = reader.readUntil('>', startSeq);
        lastUsed = endSeq;

        return record_view {
            .id  = reader.string_view(startId, endId),
            .seq = reader.string_view(startSeq, endSeq),
        };
    }
};

template <typename Reader>
fasta_reader(Reader&& reader) -> fasta_reader<io3::buffered_reader<Reader>>;

template <reader_and_dropper_c Reader>
fasta_reader(Reader&& reader) -> fasta_reader<Reader>;


}
