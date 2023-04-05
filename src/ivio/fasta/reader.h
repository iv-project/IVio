#pragma once

#include "../reader_base.h"
#include "record.h"

#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>

namespace ivio::fasta {

struct reader : public reader_base<reader> {
    using record      = fasta::record;
    using record_view = fasta::record_view;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

        // This is only relevant if a stream is being used
        bool compressed{};
    };

public:
    reader(config const& config_);
    ~reader();

    auto next() -> std::optional<record_view>;
    void close();

    auto tell() const -> size_t;
    void seek(size_t offset);

    auto tell_faidx() const -> size_t; // tell position usable for faidx
    auto read_faidx(size_t offset) -> std::string_view; // Seek to a position, compatible with faidx
};

}
