#pragma once

#include "fasta_reader_meta.h"

#include <filesystem>

namespace io3 {

struct fasta_reader_pimpl;

struct fasta_reader_config {
    std::filesystem::path input;
};

struct fasta_reader {
    using record_view = fasta::record_view;
    using iter        = fasta::iter;

private:
    std::unique_ptr<fasta_reader_pimpl> pimpl;

public:
    fasta_reader(fasta_reader_config config);
    ~fasta_reader();

    friend auto begin(fasta_reader& reader) -> iter;
    friend auto end(fasta_reader& reader) {
        return nullptr;
    }
};

}
