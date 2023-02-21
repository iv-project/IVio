#include "../fasta-benchmarks/src/read/dna5_rank_view.h"
#include "Result.h"

#include <filesystem>
#include <seqan/vcf_io.h>

using namespace seqan;

auto seqan2_bench(std::filesystem::path file) -> Result {
    Result result;

    VcfFileIn fileIn(file.c_str());

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
