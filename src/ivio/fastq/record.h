// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>
#include <string_view>

namespace ivio::fastq {

struct record;

struct record_view {
    std::string_view id;
    std::string_view seq;
    std::string_view id2;
    std::string_view qual;

    operator record() const;
    auto operator<=>(record_view const&) const = default;
};

struct record {
    std::string id;
    std::string seq;
    std::string id2;
    std::string qual;

    operator record_view() const;
    auto operator<=>(record const&) const = default;
};

// Implementation of the convert operators
inline record_view::operator record() const {
    return {
        .id   = std::string{id},
        .seq  = std::string{seq},
        .id2  = std::string{id2},
        .qual = std::string{qual},
    };
}
inline record::operator record_view() const {
    return {
        .id   = id,
        .seq  = seq,
        .id2  = id2,
        .qual = qual,
    };
}

}

// Specialization to describe their common types
template <>
struct std::common_type<ivio::fastq::record, ivio::fastq::record_view> {
    using type = ivio::fastq::record;
};

template <>
struct std::common_type<ivio::fastq::record_view, ivio::fastq::record> {
    using type = ivio::fastq::record;
};
