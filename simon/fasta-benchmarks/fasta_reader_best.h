#pragma once

#include "fasta_reader_view.h"
#include "fasta_reader_mmap.h"
#include "fasta_reader_mmap2.h"


struct fasta_reader_best {
    std::filesystem::path file;

    using Readers = std::variant<fasta_reader_mmap<mmap_file_reader>,
                                 fasta_reader_view<zlib_mmap_reader>,
                                 fasta_reader_mmap2<zlib_mmap_reader>>;
    Readers reader = [&]() -> Readers {
        auto filename = file.string();
        if (file.extension() == ".fa") {
            return fasta_reader_mmap{mmap_file_reader(filename.c_str())};
        } else if (file.extension() == ".gz") {
            auto fastReader = fasta_reader_mmap2{zlib_mmap_reader{filename.c_str()}};
            if (fastReader.valid()) {
                return fastReader;
            }
            return fasta_reader_view{zlib_mmap_reader(filename.c_str())};
        }
        throw std::runtime_error("unknown file extension");
    }();

    using record_view = fasta_reader_view_record_view;
    using iter = fasta_reader_view_iter;

    friend auto begin(fasta_reader_best& reader) {
        return iter{std::visit([](auto& reader) -> std::function<std::optional<fasta_reader_view_record_view>()>{
            return [&reader]() { return reader.next(); };
        }, reader.reader)};
    }
    friend auto end(fasta_reader_best const&) {
        return nullptr;
    }


};

