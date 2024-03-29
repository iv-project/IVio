// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "../detail/compare.h"

#include <string>
#include <string_view>

namespace ivio::faidx {

struct record;

struct record_view {
    std::string_view id;
    size_t length;
    size_t offset;
    size_t linebases;
    size_t linewidth;

    operator record() const;
    auto operator<=>(record_view const&) const = default;
};

struct record {
    std::string id;
    size_t length;
    size_t offset;
    size_t linebases;
    size_t linewidth;

    operator record_view() const;
    auto operator<=>(record const&) const = default;
};


// Implementation of the convert operators
inline record_view::operator record() const {
    return {
        .id        = std::string{id},
        .length    = length,
        .offset    = offset,
        .linebases = linebases,
        .linewidth = linewidth,
    };
}
inline record::operator record_view() const {
    return {
        .id        = id,
        .length    = length,
        .offset    = offset,
        .linebases = linebases,
        .linewidth = linewidth,
    };
}

}

// Specialization to describe their common types
template <>
struct std::common_type<ivio::faidx::record, ivio::faidx::record_view> {
    using type = ivio::faidx::record;
};

template <>
struct std::common_type<ivio::faidx::record_view, ivio::faidx::record> {
    using type = ivio::faidx::record;
};
