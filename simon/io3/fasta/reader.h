#pragma once

#include "meta.h"

#include <filesystem>
#include <functional>
#include <istream>
#include <variant>

namespace io3::fasta {

struct reader_pimpl;

struct reader_config {
    // Source: file or stream
    std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

    // This is only relevant if a stream is being used
    bool compressed{};
};

struct reader {
    using record_view = fasta::record_view;
    using iter        = fasta::iter;

private:
    std::unique_ptr<reader_pimpl> pimpl;

public:
    reader(reader_config config);
    ~reader();

    friend auto begin(reader& reader) -> iter;
    friend auto end(reader& reader) {
        return nullptr;
    }
};

}
