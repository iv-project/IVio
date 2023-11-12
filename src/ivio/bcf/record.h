// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace ivio::bcf {

struct record;

struct record_view {
    int32_t                  chromId;
    int32_t                  pos;
    int32_t                  rlen;
    std::optional<float>     qual;
    uint16_t                 n_info;
    uint16_t                 n_allele;
    uint32_t                 n_sample; // Only the first 24bit can be used
    uint8_t                  n_fmt;
    std::string_view         id;
    std::string_view         ref;
    std::span<uint8_t const> alt;
    std::span<uint8_t const> filter;
    std::span<uint8_t const> info;
    std::span<uint8_t const> format;

    operator record() const;
    auto operator<=>(record_view const& _rhs) const
    //!WORKAROUND = default doesn't work for clang
    #if !__clang__
        = default
    #endif
    ;
};

struct record {
    int32_t                  chromId;
    int32_t                  pos;
    int32_t                  rlen;
    std::optional<float>     qual;
    uint16_t                 n_info;
    uint16_t                 n_allele;
    uint32_t                 n_sample; // Only the first 24bit can be used
    uint8_t                  n_fmt;
    std::string              id;
    std::string              ref;
    std::vector<uint8_t>     alt;
    std::vector<uint8_t>     filter;
    std::vector<uint8_t>     info;
    std::vector<uint8_t>     format;

    operator record_view() const;
    auto operator<=>(record const&) const = default;
};


// Implementation of the convert operators
inline record_view::operator record() const {
    return {
        .chromId  = chromId,
        .pos      = pos,
        .rlen     = rlen,
        .qual     = qual,
        .n_info   = n_info,
        .n_allele = n_allele,
        .n_sample = n_sample,
        .n_fmt    = n_fmt,
        .id       = std::string{id},
        .ref      = std::string{ref},
        .alt      = std::vector(begin(alt), end(alt)),
        .filter   = std::vector(begin(filter), end(filter)),
        .info     = std::vector(begin(info), end(info)),
        .format   = std::vector(begin(format), end(format)),
    };
}
inline record::operator record_view() const {
    return {
        .chromId  = chromId,
        .pos      = pos,
        .rlen     = rlen,
        .qual     = qual,
        .n_info   = n_info,
        .n_allele = n_allele,
        .n_sample = n_sample,
        .n_fmt    = n_fmt,
        .id       = id,
        .ref      = ref,
        .alt      = alt,
        .filter   = filter,
        .info     = info,
        .format   = format,
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
struct std::common_type<ivio::bcf::record, ivio::bcf::record_view> {
    using type = ivio::bcf::record;
};

template <>
struct std::common_type<ivio::bcf::record_view, ivio::bcf::record> {
    using type = ivio::bcf::record;
};
