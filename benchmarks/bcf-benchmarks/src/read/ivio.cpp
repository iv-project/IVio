#include "Result.h"
#include "../fasta-benchmarks/src/read/dna5_rank_view.h"

#include <ivio/bcf/reader.h>

auto ivio_bench(std::filesystem::path file, size_t threadNbr) -> Result {
    Result result;
    for (auto && view : ivio::bcf::reader{{.input = file, .threadNbr = threadNbr}}) {
        result.l += 1;
        result.ct += view.pos;
        for (auto c : view.ref | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
        result.bytes += view.ref.size();
    }
    return result;
}
