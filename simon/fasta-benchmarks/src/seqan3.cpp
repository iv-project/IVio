#include "Result.h"

#include <seqan3/io/sequence_file/all.hpp>

auto seqan3_bench(std::string_view _file) -> Result {
    Result result;

    std::filesystem::path fasta_file{_file};
    auto reader = seqan3::sequence_file_input{fasta_file};

    //!TODO this should really be [id, seq, qual], seqan3 is backwards
    for (auto && [seq, id, qual] : reader) {
        for (auto c : seq) {
            result.ctChars[c.to_rank()] += 1;
        }
    }
    return result;
}
