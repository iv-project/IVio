#pragma once

#include "../reader_base.h"
#include "record.h"

#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>

namespace ivio::sam {

struct reader : public reader_base<reader> {
    using record      = sam::record;
    using record_view = sam::record_view;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;
    };

public:
    reader(config const& config_);
    ~reader();

    std::vector<std::string> header;

    auto next() -> std::optional<record_view>;
    void close();
};

}
