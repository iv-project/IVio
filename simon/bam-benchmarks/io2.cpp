#include "io2/sam_io_reader.h"

#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/argument_parser/all.hpp>
#include <seqan3/core/debug_stream.hpp>
#include <seqan3/io/sequence_file/all.hpp>
#include <seqan3/io/record.hpp>
#include <seqan3/io/sam_file/input.hpp>
#include <filesystem>
#include <sstream>

template <typename... Ts>
void noOpt(Ts&&...) {
    asm("");
}


void io2_bench(std::string const& _file) {

    std::filesystem::path file{_file};

    size_t a{};
    auto fin = io2::sam_io::reader{
        .input = file
    };
    for (auto && record : fin) {
        for (auto c : record.id) {
            a += c;
        }
        for (auto c : record.seq) {
            a+= c.to_rank();
        }
/*        for (auto c : record.cigar_sequence()) {
            a += c.to_rank();
        }*/
    }
    std::cout << "total: " << a << "\n";
}
