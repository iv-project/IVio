#include "Result.h"

#include <bio/seq_io/reader.hpp>

auto bio_bench(std::string_view _file) -> Result {

    std::filesystem::path fasta_file{_file};

    Result result;

    auto fin  = bio::seq_io::reader{fasta_file};
    for (auto && [id, seq, qual] : fin) {
        for (auto c : seq) {
            result.ctChars[c.to_rank()] += 1;
        }
    }
    return result;
}
