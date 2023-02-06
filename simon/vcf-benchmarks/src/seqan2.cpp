#include "../../fasta-benchmarks/src/dna5_rank_view.h"
#include "Result.h"

#include <seqan/vcf_io.h>

using namespace seqan;

auto seqan2_bench(std::string_view _file) -> Result {
    Result result;

    auto file = std::string{_file};
    VcfFileIn fileIn(toCString(file));

    VcfHeader header;
    readHeader(header, fileIn);

    VcfRecord record;
    while (!atEnd(fileIn)) {
        result.l += 1;
        readRecord(record, fileIn);
        result.ct += record.beginPos+1;
        for (auto c : record.ref | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
        result.bytes += length(record.ref);
    }

    return result;
}
