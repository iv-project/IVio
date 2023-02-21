#include "Result.h"
#include "io2/sam_io_reader.h"

#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/argument_parser/all.hpp>
#include <seqan3/core/debug_stream.hpp>
#include <seqan3/io/sequence_file/all.hpp>
#include <seqan3/io/record.hpp>
#include <seqan3/io/sam_file/input.hpp>
#include <filesystem>
#include <sstream>

auto io2_bench(std::filesystem::path file) -> Result {
    Result result;

    auto fin = io2::sam_io::reader{
        .input = file,
    };
    for (auto && record : fin) {
        for (auto c : record.seq) {
            result.ctChars[c.to_rank()] += 1;
        }
    }
    std::swap(result.ctChars[3], result.ctChars[4]);
    return result;
}
