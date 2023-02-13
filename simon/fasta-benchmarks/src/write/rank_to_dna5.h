#pragma once

#include <array>
#include <cstdint>
#include <span>

static void rank_to_dna5(std::span<uint8_t const> in, std::span<char> out) {
    constexpr static auto ccmap = []() {
        std::array<char, 256> c;
        c[0] = 'A';
        c[1] = 'C';
        c[2] = 'G';
        c[3] = 'T';
        c[4] = 'N';
        return c;
    }();

    for (size_t i{0}; i < in.size(); ++i) {
        out[i] = ccmap[in[i]];
    }
}
