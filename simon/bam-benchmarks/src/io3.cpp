#include "Result.h"
#include "../../fasta-benchmarks/src/dna5_rank_view.h"

#include <io3/bam/reader.h>

#include <cassert>
#include <thread>
#include <chrono>
#if 0
static constexpr auto dna5_rank_view = [](char c) {
    constexpr static auto ccmap = []() {
        std::array<uint8_t, 256> c;
        c.fill(0xff);
        c['A'] = 0;
        c['C'] = 1;
        c['G'] = 2;
        c['T'] = 3;
        c['N'] = 4;
        c['a'] = 0;
        c['c'] = 1;
        c['g'] = 2;
        c['t'] = 3;
        c['n'] = 4;
        return c;
    }();

    auto rank = ccmap[reinterpret_cast<uint8_t&>(c)];
/*    if (rank == 0xff) {
        throw "invalid character";
    }*/
    return rank;
};

#endif
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

auto io3_bench(std::string_view _file, size_t threadNbr) -> Result {
    auto file = std::string{_file};

    Result result;
/*    for (auto && record : io3::bam::reader{{file, threadNbr}}) {
        for (size_t i{0}; i < record.seq.size/2; ++i) {
            auto [c1, c2] = ccmap2[record.seq.data[i]];
            result.ctChars[c1] += 1;
            result.ctChars[c2] += 1;
        }
        if (record.seq.size % 2) {
            auto [c1, c2] = ccmap2[record.seq.data.back()];
            result.ctChars[c1] += 1;
        }
    }*/

    std::array<size_t, 16> ctChars{};
    for (auto record : io3::bam::reader{{file, threadNbr}}) {
        for (auto c : record.seq) {
            ctChars[c] += 1;
        }
    }
    result.ctChars[0] = ctChars[io3::bam::record_view::char_to_rank['A']];
    result.ctChars[1] = ctChars[io3::bam::record_view::char_to_rank['C']];
    result.ctChars[2] = ctChars[io3::bam::record_view::char_to_rank['G']];
    result.ctChars[3] = ctChars[io3::bam::record_view::char_to_rank['T']];
    result.ctChars[4] = ctChars[io3::bam::record_view::char_to_rank['N']];

    return result;
}
