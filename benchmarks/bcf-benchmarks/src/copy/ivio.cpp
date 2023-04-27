// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file.
// -----------------------------------------------------------------------------------------------------
#include <ivio/bcf/reader.h>
#include <ivio/bcf/writer.h>

void ivio_bench(std::filesystem::path pathIn, std::filesystem::path pathOut, size_t threadNbr) {
    auto reader = ivio::bcf::reader{{.input     = pathIn,
                                     .threadNbr = threadNbr}};
    auto writer = ivio::bcf::writer{{.output = pathOut,
                                     .header = reader.header() }};

    for (auto record_view : reader) {
        writer.write(record_view);
    }
}
