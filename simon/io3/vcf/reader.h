#pragma once

#include "../reader_base.h"
#include "record.h"

#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>

namespace io3::vcf {

struct reader : public reader_base<reader> {
    using record_view = vcf::record_view;

    std::vector<std::tuple<std::string, std::string>> header;
    std::vector<std::string> genotypes;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

        // This is only relevant if a stream is being used
        bool compressed{};
    };

public:
    reader(config const& config_);

    ~reader();

    bool readHeaderLine();
    void readHeader();

    auto next() -> std::optional<record_view>;
};

}
