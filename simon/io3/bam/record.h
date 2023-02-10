#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace io3::bam {

struct record_view {
    constexpr static auto rank_to_char = std::string_view{"=ACMGRSVTWYHKDBN"};
    constexpr static auto char_to_rank = []() {
        auto r = std::array<uint8_t, 256>{};
        r.fill(15);
        for (size_t i{0}; i < rank_to_char.size(); ++i) {
            r[rank_to_char[i]] = i;
        }
        return r;
    }();
    constexpr static auto ccmap = std::string_view{"=ACMGRSVTWYHKDBN"};
    inline static auto const ccmap2 = []() {
        auto values = std::array<std::tuple<char, char>, 256>{};

        for (size_t i{0}; i < ccmap.size(); ++i) {
            for (size_t j{0}; j < ccmap.size(); ++j) {
                values[j+i*16] = {i, j};
            }
        }
        return values;
    }();

    struct compact_seq {
        std::string_view data; // Data compressed as described in bam
        size_t           size; // Number of elements stored in the sequence
        auto operator[](size_t i) const {
            auto c = reinterpret_cast<uint8_t const&>(data[i/2]);
            auto [a, b] = ccmap2[c];
            if (i % 2 == 0) {
                return a;
            }
            return b;
        }
    };
    int32_t                     refID;
    int32_t                     pos;
    uint8_t                     mapq;
    uint16_t                    bin;
    uint16_t                    flag;
    int32_t                     next_refID;
    int32_t                     next_pos;
    int32_t                     tlen;
    std::string_view            read_name;
    std::string_view            cigar;
    compact_seq                 seq;
    std::string_view            qual;
};

}
