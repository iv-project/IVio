// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file.
// -----------------------------------------------------------------------------------------------------
#include "Result.h"

#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/io/sam_file/input.hpp>
#include <filesystem>

auto seqan3_bench(std::filesystem::path file, size_t threadNbr) -> Result {
    Result result;

    seqan3::contrib::bgzf_thread_count = threadNbr;
    auto fin = seqan3::sam_file_input{file};
    for (auto && record : fin) {
        for (auto c : record.sequence()) {
            result.ctChars[c.to_rank()] += 1;
        }
    }
    std::swap(result.ctChars[3], result.ctChars[4]);
    return result;
}
