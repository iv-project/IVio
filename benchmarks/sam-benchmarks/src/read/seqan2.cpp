#include "../fasta-benchmarks/src/read/dna5_rank_view.h"
#include "Result.h"

#include <filesystem>
#include <seqan/bam_io.h>

using namespace seqan;

auto seqan2_bench(std::filesystem::path file) -> Result {
    Result result;

    BamFileIn fileIn(file.c_str());

    BamHeader header;
    readHeader(header, fileIn);

    BamAlignmentRecord record;
    while(!atEnd(fileIn)) {
        readRecord(record, fileIn);
        for (auto c : record.seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
