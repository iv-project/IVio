#pragma once

#include "bgzf_reader.h"
#include "buffered_reader.h"
#include "file_reader.h"
#include "vcf_reader.h"

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
        if (size2 < 32+4) throw "something went worng reaing bcf file (4)";
        auto pos      = io3::bgzfUnpack<int32_t>(ptr2 + 12)+1;
        auto rlen     = io3::bgzfUnpack<int32_t>(ptr2 + 16);
//        auto qual = io3::bgzfUnpack<float>(ptr2 + 20);
        auto n_info   = io3::bgzfUnpack<int16_t>(ptr2 + 24);
        auto n_allele = io3::bgzfUnpack<int16_t>(ptr2 + 26);
        auto n_sample = io3::bgzfUnpack<int32_t>(ptr2 + 28) & 0x00ffffff;
        auto n_fmt    = io3::bgzfUnpack<uint8_t>(ptr2 + 31);

        auto readInt = [&](size_t o) -> std::tuple<int32_t, size_t> {
            auto v = io3::bgzfUnpack<uint8_t>(ptr2 + o);
            auto t = v & 0x0f;
            if (t == 1) {
                return {io3::bgzfUnpack<int8_t>(ptr2 + o + 1), o+2};
            } else if (t == 2) {
                return {io3::bgzfUnpack<int16_t>(ptr2 + o + 1), o+3};
            } else if (t == 3) {
                return {io3::bgzfUnpack<int32_t>(ptr2 + o + 1), o+5};
            } else {
                throw "BCF error, expected an int";
            }
        };
        auto readString = [&](size_t o) -> std::tuple<std::string_view, size_t> {
            auto v = io3::bgzfUnpack<uint8_t>(ptr2 + o);
            auto t = v & 0x0f;
            auto l = v >> 4;
            if (l == 15) {
                auto [i, o2] = readInt(o+1);
                return {{ptr2+o2, ptr2+o2+i}, o2+i};
            }
            return {{ptr2+o+1, ptr2+o+1+l}, o+1+l};
        };
        auto [id, o2] = readString(32);
        auto [ref, o3] = readString(o2);

        lastUsed = l_shared + l_indiv + 8;
        return record_view {
            .chrom   = reader.string_view(0, 0),
            .pos     = pos,
            .id      = id,
            .ref     = ref,
            .alt     = reader.string_view(0, 0),
            .qual    = 0.,
            .filter  = reader.string_view(0, 0),
            .info    = reader.string_view(0, 0),
            .format  = reader.string_view(0, 0),
            .samples = reader.string_view(0, 0),
        };
    }
};

template <typename Reader>
bcf_reader(Reader&& reader) -> bcf_reader<io3::buffered_reader<Reader, 1<<16>>;

template <reader_and_dropper_c Reader>
bcf_reader(Reader&& reader) -> bcf_reader<Reader>;

static_assert(record_reader_c<bcf_reader<io3::buffered_reader<io3::file_reader>>>);
}
