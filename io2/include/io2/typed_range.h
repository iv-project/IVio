#pragma once

#include "any_iter.h"

#include <ranges>

namespace io2 {

template <typename T>
struct typed_range {
    detail::any_iter<T> begin_;
    detail::any_iter<T> end_;
    size_t size_;

    typed_range() = default;

    template <typename rng_t>
    typed_range(rng_t&& rng)
        : begin_{std::ranges::begin(rng)}
        , end_{std::ranges::end(rng)}
        , size_{std::ranges::size(rng)}
    {}

    auto begin() const {
        return begin_;
    }
    auto end() const {
        return end_;
    }
    auto size() const {
        return size_;
    }

    friend auto begin(typed_range const& tr) {
        return tr.begin_;
    };

    friend auto end(typed_range const& tr) {
        return tr.end_;
    }
};

}
