#pragma once

#include <string>
#include <string_view>

namespace ivio::fasta_idx {

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
struct std::common_type<ivio::fasta_idx::record, ivio::fasta_idx::record_view> {
    using type = ivio::fasta_idx::record;
};

template <>
struct std::common_type<ivio::fasta_idx::record_view, ivio::fasta_idx::record> {
    using type = ivio::fasta_idx::record;
};
