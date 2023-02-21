#include "Result.h"

#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/io/sam_file/input.hpp>
#include <filesystem>

auto seqan3_bench(std::filesystem::path file) -> Result {
    Result result;

    auto fin = seqan3::sam_file_input{file};
    for (auto && record : fin) {
        for (auto c : record.sequence()) {
            result.ctChars[c.to_rank()] += 1;
        }
    }
    std::swap(result.ctChars[3], result.ctChars[4]);
    return result;
}
