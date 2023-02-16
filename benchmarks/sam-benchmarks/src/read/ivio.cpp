#include "Result.h"
#include "../fasta-benchmarks/src/read/dna5_rank_view.h"

#include <ivio/sam/reader.h>

auto ivio_bench(std::string_view _file) -> Result {
    auto file = std::string{_file};

    Result result;
    for (auto && record : ivio::sam::reader{{file}}) {
        for (auto c : record.seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
