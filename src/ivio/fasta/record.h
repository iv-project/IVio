// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "../detail/compare.h"

#include <string>
#include <string_view>

namespace ivio::fasta {

struct record;

struct record_view {
    std::string_view id;
    std::string_view seq;

    operator record() const;
    auto operator<=>(record_view const&) const = default;
};

struct record {
    std::string id;
    std::string seq;

    operator record_view() const;
    auto operator<=>(record const&) const = default;
};


// Implementation of the convert operators
inline record_view::operator record() const {
    return {
        .id  = std::string{id},
        .seq = std::string{seq},
    };
}
inline record::operator record_view() const {
    return {
        .id  = id,
        .seq = seq,
    };
}

}

// Specialization to describe their common types
template <>
struct std::common_type<ivio::fasta::record, ivio::fasta::record_view> {
    using type = ivio::fasta::record;
};

template <>
struct std::common_type<ivio::fasta::record_view, ivio::fasta::record> {
    using type = ivio::fasta::record;
};
