#pragma once

#include <string_view>

namespace io3::fasta {

struct record_view {
    std::string_view id;
    std::string_view seq;
};

}
