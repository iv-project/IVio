// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file.
// -----------------------------------------------------------------------------------------------------
#include "../fasta-benchmarks/src/read/dna5_rank_view.h"
#include "Result.h"

#include <filesystem>
#include <seqan/bam_io.h>

using namespace seqan;

auto seqan2_bench(std::filesystem::path file, size_t threadNbr) -> Result {
    Result result;

    BamFileIn fileIn(file.c_str());

    BamHeader header;
    readHeader(header, fileIn);

    BamAlignmentRecord record;
    while(!atEnd(fileIn)) {
        readRecord(record, fileIn);
        for (auto c : record.seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
