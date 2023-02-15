#pragma once

#include <cstddef>
#include <string_view>

namespace io3::sam {

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

    record() = default;
    record(record_view v)
        : qname{v.qname}
        , flag {v.flag}
        , rname{v.rname}
        , pos  {v.pos}
        , mapq {v.mapq}
        , cigar{v.cigar}
        , rnext{v.rnext}
        , pnext{v.pnext}
        , tlen {v.tlen}
        , seq  {v.seq}
        , qual {v.qual}
    {}
    operator record_view() const {
        return record_view {
            qname,
            flag,
            rname,
            pos,
            mapq,
            cigar,
            rnext,
            pnext,
            tlen,
            seq,
            qual
        };
    }
};

}
