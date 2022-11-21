#pragma once

#include "meta.h"

#include <filesystem>
#include <istream>
#include <variant>

namespace io3::fasta {

struct reader_pimpl;

struct reader_config {
    std::variant<std::filesystem::path, std::istream> input;
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
