#pragma once

#include <array>
#include <cstdint>
#include <ranges>

inline constexpr auto dna5_rank_view = std::views::transform([](char c) -> uint8_t {
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
    if (rank == 0xff) {
        throw "invalid character";
    }
    return rank;
});
