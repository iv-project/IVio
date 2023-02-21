#include "../fasta-benchmarks/src/read/dna5_rank_view.h"
#include "Result.h"

#include <ivio/bam/reader.h>

#include <cassert>
#include <thread>
#include <chrono>
constexpr static auto ccmap = std::string_view{"=ACMGRSVTWYHKDBN"};
static auto const ccmap2 = []() {
    auto values = std::array<std::tuple<char, char>, 256>{};

    auto dna5_rank_view = [](char v) {
        if (v == 'A') return 0;
        if (v == 'C') return 1;
        if (v == 'G') return 2;
        if (v == 'T') return 3;
        if (v == 'N') return 4;
        return 0xff;
    };

    for (size_t i{0}; i < ccmap.size(); ++i) {
        for (size_t j{0}; j < ccmap.size(); ++j) {
            values[j+i*16] = {dna5_rank_view(ccmap[i]), dna5_rank_view(ccmap[j])};
        }
    }
    return values;
}();

auto ivio_bench(std::filesystem::path file, size_t threadNbr) -> Result {
    Result result;
    std::array<size_t, 16> ctChars{};
    for (auto record : ivio::bam::reader{{file, threadNbr}}) {
        for (auto c : record.seq) {
            ctChars[c] += 1;
        }
    }
    result.ctChars[0] = ctChars[ivio::bam::record_view::char_to_rank['A']];
    result.ctChars[1] = ctChars[ivio::bam::record_view::char_to_rank['C']];
    result.ctChars[2] = ctChars[ivio::bam::record_view::char_to_rank['G']];
    result.ctChars[3] = ctChars[ivio::bam::record_view::char_to_rank['T']];
    result.ctChars[4] = ctChars[ivio::bam::record_view::char_to_rank['N']];

    return result;
}
