#include "Result.h"
#include "dna5_rank_view.h"

#include <io3/fasta/reader.h>

auto io3_bench(std::filesystem::path file) -> Result {
    Result result;

    auto reader = io3::fasta::reader{{.input = file}};
    for (auto && [id, seq] : reader) {
        for (auto c : seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
