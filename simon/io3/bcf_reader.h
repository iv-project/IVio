#pragma once

#include "bgzf_reader.h"
#include "buffered_reader.h"
#include "file_reader.h"
#include "vcf_reader.h"

#include <functional>

namespace io3 {

template <typename Reader>
struct bcf_reader {
    Reader reader;
    size_t lastUsed{};

    using record_view = vcf_reader_view_record_view;
    using iter        = vcf_reader_view_iter;

    template <typename R>
    bcf_reader(R&& r)
        : reader{std::forward<R>(r)}
    {
        readHeader();
    }


    bcf_reader(bcf_reader const&) = delete;
    bcf_reader(bcf_reader&& _other) noexcept = default;
    ~bcf_reader() = default;


    friend auto begin(bcf_reader& reader) {
        return iter{[&reader]() { return reader.next(); }};
    }
    friend auto end(bcf_reader const&) {
        return nullptr;
    }

    std::vector<std::string> header;
    std::string tableHeader;

    void readHeader() {
        auto [ptr, size] = reader.read(9);
        if (size < 9) throw "something went wrong reading bcf file (1)";

        size_t txt_len = io3::bgzfUnpack<uint32_t>(ptr + 5);
        reader.read(9 + txt_len); // read complete header !TODO safe header for future processing
        reader.dropUntil(9 + txt_len);
    }

    auto next() -> std::optional<record_view> {
        reader.dropUntil(lastUsed);
        auto [ptr, size] = reader.read(8);
        if (size == 0) return std::nullopt;
        if (size < 8) throw "something went wrong reading bcf file (2)";

        auto l_shared = io3::bgzfUnpack<uint32_t>(ptr+0);
        auto l_indiv  = io3::bgzfUnpack<uint32_t>(ptr+4);
        auto flen = l_shared + l_indiv + 8;
        auto [ptr2, size2] = reader.read(flen);
        if (size2 < flen) throw "something went wrong reading bcf file (3)";
        if (size2 < 12+4) throw "something went worng reaing bcf file (4)";
        auto pos = io3::bgzfUnpack<int32_t>(ptr2 + 12)+1;

        lastUsed = l_shared + l_indiv + 8;
        return record_view {
            . chrom   = reader.string_view(0, 0),
            . pos     = pos,
            . id      = reader.string_view(0, 0),
            . ref     = reader.string_view(0, 0),
            . alt     = reader.string_view(0, 0),
            . filter  = reader.string_view(0, 0),
            . info    = reader.string_view(0, 0),
            . format  = reader.string_view(0, 0),
            . samples = reader.string_view(0, 0),
        };
    }
};

template <typename Reader>
bcf_reader(Reader&& reader) -> bcf_reader<io3::buffered_reader<Reader, 1<<16>>;

template <reader_and_dropper_c Reader>
bcf_reader(Reader&& reader) -> bcf_reader<Reader>;

static_assert(record_reader_c<bcf_reader<io3::buffered_reader<io3::file_reader>>>);
}
