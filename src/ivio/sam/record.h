#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace ivio::sam {

struct record;

struct record_view {
    std::string_view            qname;
    int32_t                     flag;
    std::string_view            rname;
    int32_t                     pos;
    int32_t                     mapq;
    std::string_view            cigar;
    std::string_view            rnext;
    int32_t                     pnext;
    int32_t                     tlen;
    std::string_view            seq;
    std::string_view            qual;

    operator record() const;
    auto operator<=>(record_view const&) const = default;
};

struct record {
    std::string            qname;
    int32_t                flag;
    std::string            rname;
    int32_t                pos;
    int32_t                mapq;
    std::string            cigar;
    std::string            rnext;
    int32_t                pnext;
    int32_t                tlen;
    std::string            seq;
    std::string            qual;

    operator record_view() const&;
    auto operator<=>(record const&) const = default;
};

// Implementation of the convert operators
inline record_view::operator record() const {
    return {
        .qname   = std::string{qname},
        .flag    = flag,
        .rname   = std::string{rname},
        .pos     = pos,
        .mapq    = mapq,
        .cigar   = std::string{cigar},
        .rnext   = std::string{rnext},
        .pnext   = pnext,
        .tlen    = tlen,
        .seq     = std::string{seq},
        .qual    = std::string{qual},
    };
}
inline record::operator record_view() const& {
    return {
        .qname   = qname,
        .flag    = flag,
        .rname   = rname,
        .pos     = pos,
        .mapq    = mapq,
        .cigar   = cigar,
        .rnext   = rnext,
        .pnext   = pnext,
        .tlen    = tlen,
        .seq     = seq,
        .qual    = qual,
    };
}

}

// Specialization to describe their common types
template <>
struct std::common_type<ivio::sam::record, ivio::sam::record_view> {
    using type = ivio::sam::record;
};

template <>
struct std::common_type<ivio::sam::record_view, ivio::sam::record> {
    using type = ivio::sam::record;
};
