#include "Result.h"

#include <bio/seq_io/reader.hpp>

auto bio_bench(std::filesystem::path file) -> Result {
    Result result;

    auto fin  = bio::seq_io::reader{file};
    for (auto && [id, seq, qual] : fin) {
        for (auto c : seq) {
            result.ctChars[c.to_rank()] += 1;
        }
    }
    std::swap(result.ctChars[3], result.ctChars[4]);
    return result;
}
