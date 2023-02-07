#include "Result.h"
#include "dna5_rank_view.h"

#include <cassert>
#include <io3/fastq/reader.h>

auto io3_bench(std::string_view _file) -> Result {
    Result result;
    auto file   = std::string{_file};

    for (auto && record : io3::fastq::reader{{.input = file}}) {
        for (auto c : record.seq | dna5_rank_view) {
            assert(c < result.ctChars.size());
            result.ctChars[c] += 1;
        }
    }
    return result;
}
