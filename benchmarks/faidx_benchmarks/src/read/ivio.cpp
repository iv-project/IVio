// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file.
// -----------------------------------------------------------------------------------------------------
#include "Result.h"
#include "dna5_rank_view.h"

#include <ivio/faidx/reader.h>
#include <ivio/fasta/reader.h>

auto ivio_bench(std::filesystem::path file) -> Result {
    Result result;

    auto idx_file = std::filesystem::path{file.string() + ".fai"};
    auto idx_reader = ivio::faidx::reader{{.input = idx_file }};
    auto reader     = ivio::fasta::reader{{.input = file }};

    for (auto const& idx : idx_reader) {
        reader.seek_faidx(idx);
        auto record = reader.next();
        assert(record);
        for (auto c : record->seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
