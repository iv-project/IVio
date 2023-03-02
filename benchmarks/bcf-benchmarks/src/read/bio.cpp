#include "Result.h"

#include <bio/io/var/reader.hpp>

auto bio_bench(std::filesystem::path file, size_t threadNbr) -> Result {
    Result result;

    auto options = bio::io::var::reader_options{};
    options.stream_options.threads = threadNbr;

    auto fin  = bio::io::var::reader{file, options};
    for (auto & r : fin) {
        result.l += 1;
        result.ct += r.pos-1;
        for (auto c : r.ref) {
            result.ctChars[c.to_rank()] += 1;
        }
        result.bytes += r.ref.size();
    }
    std::swap(result.ctChars[3], result.ctChars[4]);
    return result;
}
