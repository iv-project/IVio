#include "Result.h"

#include <seqan/seq_io.h>

using namespace seqan;

auto seqan2_bench(std::ifstream& stream, bool compressed) -> Result {
    Result result;
    if (compressed) return result;

    SeqFileIn seqFileIn(stream);

    CharString id;
    Dna5String seq;

    while(!atEnd(seqFileIn)) {
        readRecord(id, seq, seqFileIn);
        for (auto c : seq) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
