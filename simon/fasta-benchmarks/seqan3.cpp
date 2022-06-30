#include <seqan3/io/sequence_file/all.hpp>
#include "io2/seq_io_reader.h"

namespace {

template <typename Reader>
void benchmark(Reader&& reader) {
    std::array<int, 5> ctChars{};
    //!TODO this should really be [id, seq, qual], seqan3 is backwards
    for (auto && [seq, id, qual] : reader) {
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
}


void seqan3_bench(std::string const& _file) {
    std::filesystem::path fasta_file{_file};
    benchmark(seqan3::sequence_file_input{fasta_file});
}

void seqan223_bench(std::string const& _file) {
    auto reader = io2::seq_io::reader {
        .input = _file,
//        .alphabet = sgg_io::type<seqan3::dna4>, // by default dna5
    };
    benchmark(reader);
}
