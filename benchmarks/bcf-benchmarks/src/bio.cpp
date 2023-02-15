#include "Result.h"

#include <bio/io/var/reader.hpp>

auto bio_bench(std::string_view _file) -> Result {
    Result result;

    std::filesystem::path file{_file};
    auto fin  = bio::io::var::reader{file};
    for (auto & r : fin) {
        result.l += 1;
        result.ct += r.pos;
        for (auto c : r.ref) {
            result.ctChars[c.to_rank()] += 1;
        }
        result.bytes += r.ref.size();
    }
    std::swap(result.ctChars[3], result.ctChars[4]);
    return result;
}
