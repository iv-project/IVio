#include "Result.h"
#include "dna5_rank_view.h"

#include <cassert>
#include <fstream>
#include <io3/fasta/reader.h>

auto io3_bench(std::ifstream& stream, bool compressed) -> Result {
    Result result;

    auto reader = io3::fasta::reader{{.input = stream, .compressed = compressed}};
    for (auto && [id, seq] : reader) {
        for (auto c : seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
