#include <seqan/seq_io.h>

using namespace seqan;

void seqan2_bench(std::string const& file) {
    SeqFileIn seqFileIn(toCString(file));

    std::array<int, 5> ctChars{};
    CharString id;
    Dna5String seq;

    while(!atEnd(seqFileIn)) {
        readRecord(id, seq, seqFileIn);
        for (auto c : seq) {
            ctChars[c] += 1;
        }
    }

    size_t a{};
    for (size_t i{0}; i<ctChars.size(); ++i) {
        std::cout << i << ": " << ctChars[i] << "\n";
        a += ctChars[i];
    }
    std::cout << "total: " << a << "\n";
}
