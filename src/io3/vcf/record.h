#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace io3::vcf {

struct record_view {
    using string_view_list = std::span<std::string_view>;

    std::string_view            chrom;
    int32_t                     pos;
    std::string_view            id;
    std::string_view            ref;
    string_view_list            alt;
    std::optional<float>        qual;
    string_view_list            filter;
    string_view_list            info;
    string_view_list            formats;
    std::span<string_view_list> samples;
};

}
