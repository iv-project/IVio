#pragma once

#include "../reader_base.h"
#include "record.h"

#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>
#include <unordered_map>

namespace io3::bam {

struct reader : public reader_base<reader> {
    using record_view = bam::record_view;

    std::string headerBuffer;
    std::vector<std::string_view> header;
    std::string_view              tableHeader;
    std::unordered_map<std::string_view, std::vector<std::string_view>> headerMap;

    std::vector<std::string_view>& contigMap = headerMap["contig"];
    std::vector<std::string_view>& filterMap = headerMap["filter"];

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

        size_t threadNbr = 0;

        // This is only relevant if a stream is being used
        bool compressed{};
    };

public:
    reader(config const& config_);
    ~reader();

    auto next() -> std::optional<record_view>;
};

}
