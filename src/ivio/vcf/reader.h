#pragma once

#include "../reader_base.h"
#include "header.h"
#include "record.h"

#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>

namespace ivio::vcf {

struct reader : public reader_base<reader> {
    using record      = vcf::record;
    using record_view = vcf::record_view;

    vcf::header header_;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

        // This is only relevant if a stream is being used
        bool compressed{};
    };

public:
    reader(config const& config_);
    ~reader();

    auto header() const -> vcf::header const& { return header_; }

    auto next() -> std::optional<record_view>;
};

}
