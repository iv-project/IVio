#include "Result.h"

#include <bio/io/var/reader.hpp>

auto bio_bench(std::filesystem::path file, size_t threadNbr) -> Result {
    Result result;

    //!Note: this line is very fragile since bgzf_thread_count works on per
    //translation unit (error in bio, that is fixed in seqan3)
    bio::io::contrib::bgzf_thread_count = threadNbr;

    //!Note: note clear which of these options have to be set
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
