#pragma once

#include <cstddef>
#include <optional>
#include <span>
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

}
