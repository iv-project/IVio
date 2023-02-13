#include "Result.h"

#include <seqan/seq_io.h>

using namespace seqan;

auto seqan2_bench(std::string_view _file) -> Result{
    Result result;

    auto file = std::string{_file};
    SeqFileIn seqFileIn(toCString(file));

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
