#pragma once

#include <ranges>

namespace io2 {

template <typename rng_t, typename value_t>
concept range_over = std::ranges::range<rng_t> and requires(rng_t r) {
    { *r.begin() } -> std::convertible_to<value_t>;
};

}
