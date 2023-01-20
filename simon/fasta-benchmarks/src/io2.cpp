#include "io2/seq_io_reader.h"

#include <seqan3/utility/range/to.hpp>

void io2_bench(std::string const& _file) {
    auto reader = io2::seq_io::reader {
        .input = _file,
//        .alphabet = sgg_io::type<seqan3::dna4>, // by default dna5
    };

    std::array<int, 5> ctChars{};
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



void io2_copy_bench(std::string const& _file) {
    auto reader = io2::seq_io::reader {
        .input = _file,
//        .alphabet = sgg_io::type<seqan3::dna4>, // by default dna5
    };

    std::array<int, 5> ctChars{};
    for (auto && [id, seq, qual] : reader) {
        for (auto c : seq | seqan3::ranges::to<std::vector>()) {
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
