#include "Result.h"
#include "dna5_rank_view.h"

#include <cassert>
#include <fstream>
#include <ivio/fasta/reader.h>

auto ivio_bench(std::ifstream& stream, bool compressed) -> Result {
    Result result;

    auto reader = ivio::fasta::reader{{.input = stream, .compressed = compressed}};
    for (auto && [id, seq] : reader) {
        for (auto c : seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
