#include "dna5_rank_view.h"
#include "rank_to_dna5.h"

#include <cassert>
#include <fstream>
#include <vector>
#include <ivio/fasta/writer.h>

void ivio_bench(std::filesystem::path file, std::vector<std::tuple<std::string, std::vector<uint8_t>>> const& data) {
    auto writer = ivio::fasta::writer{{.output = file}};
    std::string seq_as_str;
    for (auto const& [seq_id, seq] : data) {
        seq_as_str.resize(seq.size());
        rank_to_dna5(seq, seq_as_str);
        writer.write({.id  = seq_id,
                      .seq = seq_as_str});
    }
}
