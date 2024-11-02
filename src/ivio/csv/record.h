// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "../detail/compare.h"

#include <string>
#include <vector>

namespace ivio::csv {

struct record;

struct record_view {
    std::span<std::string const> entries;
    operator record() const;
    auto operator<=>(record_view const&) const = default;
};

struct record {
    std::vector<std::string> entries;

    operator record_view() const;
    auto operator<=>(record const&) const = default;
};


// Implementation of the convert operators
inline record_view::operator record() const {
    return {
        .entries = std::vector<std::string>(begin(entries), end(entries)),
    };
}
inline record::operator record_view() const {
    return {
        .entries = entries,
    };
}

}

// Specialization to describe their common types
template <>
struct std::common_type<ivio::csv::record, ivio::csv::record_view> {
    using type = ivio::csv::record;
};

template <>
struct std::common_type<ivio::csv::record_view, ivio::csv::record> {
    using type = ivio::csv::record;
};
