#include "dna5_rank_view.h"
#include "rank_to_dna5.h"

#include <cassert>
#include <fstream>
#include <vector>
#include <io3/fasta/writer.h>

void io3_bench(std::filesystem::path file, std::vector<std::tuple<std::string, std::vector<uint8_t>>> const& data) {
    auto writer = io3::fasta::writer{{.output = file}};
    for (auto const& [seq_id, seq] : data) {
        auto seq_as_str = rank_to_dna5(seq);
        writer.write({.id  = seq_id,
                      .seq = seq_as_str});
    }
}
