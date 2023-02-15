#include "Result.h"
#include "../fasta-benchmarks/src/read/dna5_rank_view.h"

#include <ivio/vcf/reader.h>

auto ivio_bench(std::string_view _file) -> Result {
    auto file = std::string{_file};

    Result result;
    for (auto && view : ivio::vcf::reader{{file}}) {
        result.l += 1;
        result.ct += view.pos;
        for (auto c : view.ref | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
        result.bytes += view.ref.size();
    }
    return result;
}
