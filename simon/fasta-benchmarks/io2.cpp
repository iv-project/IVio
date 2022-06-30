#include "io2/seq_io_reader.h"

void io2_bench(std::string const& _file) {
    auto reader = io2::seq_io::reader {
        .input = _file,
//        .alphabet = sgg_io::type<seqan3::dna4>, // by default dna5
    };

    std::array<int, 5> ctChars{};
    //!TODO this should really be [id, seq, qual], seqan3 is backwards
    for (auto && [id, seq, qual] : reader) {
        for (auto c : seq) {
            ctChars[c.to_rank()] += 1;
        }
    }

    size_t a{};
    for (size_t i{0}; i<ctChars.size(); ++i) {
        std::cout << i << ": " << ctChars[i] << "\n";
        a += ctChars[i];
    }
    std::cout << "total: " << a << "\n";
}
