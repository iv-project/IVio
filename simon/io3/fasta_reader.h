#pragma once

#include "fasta_reader_meta.h"

#include <filesystem>

namespace io3::fasta {

struct reader_pimpl;

struct reader_config {
    std::filesystem::path input;
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
