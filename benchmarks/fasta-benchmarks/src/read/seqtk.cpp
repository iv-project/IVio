#include "Result.h"
#include "dna5_rank_view.h"

#include <kseq.h>
#include <zlib.h>
#include <filesystem>

KSEQ_INIT(gzFile, gzread)

auto seqtk_bench(std::filesystem::path file) -> Result {
    Result result;

    gzFile fp = gzopen(file.c_str(), "r");
    kseq_t* kseq = kseq_init(fp);
    while (kseq_read(kseq) >= 0) {
//        auto name = std::string_view{kseq->name.s, kseq->name.l};
        auto seq = std::string_view{kseq->seq.s, kseq->seq.l};
        for (auto c : seq | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    kseq_destroy(kseq);
    gzclose(fp);
    return result;
}
