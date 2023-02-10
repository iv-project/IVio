#include "../../fasta-benchmarks/src/dna5_rank_view.h"
#include "Result.h"

#include <seqan/bam_io.h>

using namespace seqan;

auto seqan2_bench(std::string_view _file, size_t threadNbr) -> Result {
    Result result;

    std::string file = std::string{_file};
    BamFileIn fileIn(toCString(file));

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
