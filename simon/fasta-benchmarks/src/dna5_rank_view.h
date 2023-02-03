#pragma once

#include <array>
#include <cstdint>
#include <ranges>

inline constexpr auto dna5_rank_view = std::views::transform([](char c) {
    constexpr static auto ccmap = []() {
        std::array<uint8_t, 256> c;
        c.fill(0xff);
        c['A'] = 0;
        c['C'] = 1;
        c['G'] = 2;
        c['N'] = 3;
        c['T'] = 4;
        c['a'] = 0;
        c['c'] = 1;
        c['g'] = 2;
        c['n'] = 3;
        c['t'] = 4;
        return c;
    }();

    auto rank = ccmap[reinterpret_cast<uint8_t&>(c)];
    if (rank == 0xff) {
        throw "invalid character";
    }
    return rank;
});
