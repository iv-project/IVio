#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace io3::bam {

struct record_view {
    int32_t                     refID;
    int32_t                     pos;
    uint8_t                     mapq;
    uint16_t                    bin;
    uint16_t                    flag;
    int32_t                     next_refID;
    int32_t                     next_pos;
    int32_t                     tlen;
    std::string_view            read_name;
    std::string_view            cigar;
    std::string_view            seq;
    std::string_view            qual;
};

}
