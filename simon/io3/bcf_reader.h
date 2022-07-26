#pragma once

#include "bgzf_reader.h"
#include "buffered_reader.h"

#include <functional>

namespace io3 {

template <typename Reader>
struct bcf_reader {
    Reader reader;
    size_t inPos{};

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
        auto [ptr, size] = reader.read(9 + inPos);
        if (size < 9) throw "something went wrong reading bcf file (1)";

        size_t txt_len = io3::bgzfUnpack<uint32_t>(ptr + 5 + inPos);
        reader.read(9 + txt_len + inPos); // read complete header !TODO safe header for future processing
        inPos = reader.dropUntil(9 + txt_len + inPos);
    }

    auto next() -> std::optional<record_view> {
        inPos = reader.dropUntil(inPos);
        auto [ptr, size] = reader.read(8 + inPos);
        if (size-inPos == 0) return std::nullopt;
        if (size-inPos < 8) throw "something went wrong reading bcf file (2)";

        auto l_shared = io3::bgzfUnpack<uint32_t>(ptr+0 + inPos);
        auto l_indiv  = io3::bgzfUnpack<uint32_t>(ptr+4 + inPos);
        auto flen = l_shared + l_indiv + 8;
        auto [ptr2, size2] = reader.read(flen + inPos);
        if (size2 < flen + inPos) throw "something went wrong reading bcf file (3)";
        if (size2 < 12+inPos+4) throw "something went worng reaing bcf file (4)";
        auto pos = io3::bgzfUnpack<int32_t>(ptr2 + 12 + inPos)+1;

        inPos = l_shared + l_indiv + 8 + inPos;
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


}
