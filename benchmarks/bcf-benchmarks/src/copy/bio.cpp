// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file.
// -----------------------------------------------------------------------------------------------------
#include <bio/io/var/reader.hpp>
#include <bio/io/var/writer.hpp>

void bio_bench(std::filesystem::path pathIn, std::filesystem::path pathOut, size_t threadNbr) {

    auto options = bio::io::var::reader_options{};
    options.stream_options.threads = std::max(threadNbr, size_t{1});

    auto fin  = bio::io::var::reader{pathIn, options};
    auto fout = bio::io::var::writer{pathOut};

    for (auto & rec : fin) {
        fout.push_back(rec);
    }
}
