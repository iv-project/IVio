#include "Result.h"
#include "../../fasta-benchmarks/src/dna5_rank_view.h"

#include <io3/bam/reader.h>

auto io3_bench(std::string_view _file) -> Result {
    auto file = std::string{_file};

    Result result;
    for (auto && record : io3::bam::reader{{file}}) {
        for (auto c : record.seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
