#pragma once

#include "../reader_base.h"
#include "header.h"
#include "record.h"

#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>
#include <unordered_map>

namespace ivio::bam {

struct reader : public reader_base<reader> {
    using record      = bam::record;
    using record_view = bam::record_view;

    bam::header header_;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

        size_t threadNbr = 0;
    };

public:
    reader(config const& config_);
    ~reader();

    auto header() const -> bam::header const& { return header_; }

    auto next() -> std::optional<record_view>;
};

}
