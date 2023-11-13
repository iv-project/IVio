// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

namespace ivio::bam {

struct record;

struct record_view {
    constexpr static auto rank_to_char = std::string_view{"=ACMGRSVTWYHKDBN"};
    constexpr static auto char_to_rank = []() {
        auto r = std::array<uint8_t, 256>{};
        r.fill(15);
        for (size_t i{0}; i < rank_to_char.size(); ++i) {
            r[rank_to_char[i]] = i;
        }
        return r;
    }();

    struct compact_seq {
        std::span<uint8_t const> data; // Data compressed as described in bam
        size_t                   size; // Number of elements stored in the sequence
        auto operator[](size_t i) const -> uint8_t {
            auto c = data[i/2];
            if (i % 2 == 0) {
                c >>= 4;
            }
            return c & 0xf;
        }

        struct iter {
            using value_type = uint8_t;
            compact_seq const* seq;
            size_t i{};
            auto operator<=>(iter const& _other) const = default;
            auto operator*() const {
                return seq->operator[](i);
            }
            auto operator++() -> iter& {
                ++i;
                return *this;
            }
        };
        friend auto begin(compact_seq const& seq) {
            return iter{&seq, 0};
        }
        friend auto end(compact_seq const& seq) {
            return iter{&seq, seq.size};
        }
    };
    int32_t                     refID;
    int32_t                     pos;
    uint8_t                     mapq;
    uint16_t                    bin;
    uint16_t                    flag;
    int32_t                     next_refID;
    int32_t                     next_pos;
    int32_t                     tlen;
    std::string_view            read_name;
    std::span<uint8_t const>    cigar;
    compact_seq                 seq;
    std::span<uint8_t const>    qual;

    operator record() const;
    auto operator<=>(record_view const& _rhs) const
    //!WORKAROUND = default doesn't work for clang
    #if !__clang__
        = default
    #endif
    ;

};

struct record {
    int32_t                     refID;
    int32_t                     pos;
    uint8_t                     mapq;
    uint16_t                    bin;
    uint16_t                    flag;
    int32_t                     next_refID;
    int32_t                     next_pos;
    int32_t                     tlen;
    std::string                 read_name;
    std::vector<uint8_t>        cigar;
    std::vector<uint8_t>        seq;
    size_t                      seq_len;
    std::vector<uint8_t>        qual;

    operator record_view() const;
    auto operator<=>(record const&) const = default;
};

// Implementation of the convert operators
inline record_view::operator record() const {
    return {
        .refID      = refID,
        .pos        = pos,
        .mapq       = mapq,
        .bin        = bin,
        .flag       = flag,
        .next_refID = next_refID,
        .next_pos   = next_pos,
        .tlen       = tlen,
        .read_name  = std::string{read_name},
        .cigar      = std::vector(begin(cigar), end(cigar)),
        .seq        = std::vector(begin(seq.data), end(seq.data)),
        .seq_len    = seq.size,
        .qual       = std::vector(begin(qual), end(qual)),
    };
}

inline record::operator record_view() const {
    return {
        .refID      = refID,
        .pos        = pos,
        .mapq       = mapq,
        .bin        = bin,
        .flag       = flag,
        .next_refID = next_refID,
        .next_pos   = next_pos,
        .tlen       = tlen,
        .read_name  = read_name,
        .cigar      = cigar,
        .seq        = {seq, seq_len},
        .qual       = qual,
    };
}

//!WORKAROUND = default doesn't work for clang
#if __clang__
inline auto record_view::operator<=>(record_view const& _rhs) const {
    return static_cast<record>(*this) <=> static_cast<record>(_rhs);
}
#endif

}

// Specialization to describe their common types
template <>
struct std::common_type<ivio::bam::record, ivio::bam::record_view> {
    using type = ivio::bam::record;
};

template <>
struct std::common_type<ivio::bam::record_view, ivio::bam::record> {
    using type = ivio::bam::record;
};
