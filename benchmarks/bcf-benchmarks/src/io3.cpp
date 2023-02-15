#include "Result.h"
#include "../../fasta-benchmarks/src/dna5_rank_view.h"

#include <io3/bcf/reader.h>

auto io3_bench(std::string_view _file) -> Result {
    auto file = std::string{_file};

    Result result;
    for (auto && view : io3::bcf::reader{{file}}) {
        result.l += 1;
        result.ct += view.pos;
        for (auto c : view.ref | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
        result.bytes += view.ref.size();
    }
    return result;
}
