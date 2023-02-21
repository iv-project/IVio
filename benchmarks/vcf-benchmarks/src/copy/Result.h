#pragma once

#include <array>
#include <cstddef>

struct Result {
    size_t ct{};
    size_t sum{};
    size_t l{};
    size_t bytes{};
    std::array<size_t, 5> ctChars{};
};
