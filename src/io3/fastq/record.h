#pragma once

#include <string_view>

namespace io3::fastq {

struct record_view {
    std::string_view id;
    std::string_view seq;
    std::string_view id2;
    std::string_view qual;
};

}
