#include "Result.h"
#include "../fasta-benchmarks/src/read/dna5_rank_view.h"

#include <ivio/sam/reader.h>

auto ivio_bench(std::filesystem::path file) -> Result {
    Result result;
    for (auto && record : ivio::sam::reader{{file}}) {
        for (auto c : record.seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
