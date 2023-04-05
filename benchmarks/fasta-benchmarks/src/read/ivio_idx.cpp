#include "Result.h"
#include "dna5_rank_view.h"

#include <ivio/fasta_idx/reader.h>
#include <ivio/fasta/reader.h>

auto ivio_idx_bench(std::filesystem::path file) -> Result {
    Result result;

    auto idx_file = std::filesystem::path{file.string() + ".fai"};
    auto idx_reader = ivio::fasta_idx::reader{{.input = idx_file }};
    auto reader     = ivio::fasta::reader{{.input = file }};

    for (auto const& idx : idx_reader) {
        for (auto c : reader.read_faidx(idx.offset) | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
    }
    return result;
}
