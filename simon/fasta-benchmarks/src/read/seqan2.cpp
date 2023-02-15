#include "Result.h"

#include <filesystem>
#include <seqan/seq_io.h>

using namespace seqan;

auto seqan2_bench(std::filesystem::path file) -> Result{
    Result result;

    SeqFileIn seqFileIn(file.c_str());

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
