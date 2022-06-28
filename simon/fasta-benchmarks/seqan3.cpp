#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/argument_parser/all.hpp>
#include <seqan3/core/debug_stream.hpp>
#include <seqan3/io/sequence_file/all.hpp>
#include <filesystem>
#include <sstream>

template <typename... Ts>
void noOpt(Ts&&...) {
    asm("");
}


void seqan3_bench(std::string const& _file) {

    std::filesystem::path fasta_file{_file};

    std::array<int, 5> ctChars{};

    auto fin  = seqan3::sequence_file_input{fasta_file};
    for (auto && [seq, id, qual] : fin) {
        for (auto c : seq) {
            ctChars[c.to_rank()] += 1;
        }
    }
    size_t a{};
    for (int i{0}; i<ctChars.size(); ++i) {
        std::cout << i << ": " << ctChars[i] << "\n";
        a += ctChars[i];
    }
    std::cout << "total: " << a << "\n";
}
