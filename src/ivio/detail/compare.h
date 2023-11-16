// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <algorithm>
#include <compare>
#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace ivio {

//!WORKAROUND llvm < 17 does not provide some comparison operators
#if defined(_LIBCPP_VERSION) && _LIBCPP_VERSION < 170000
template< class T, class U >
    requires std::three_way_comparable_with<T, U>
constexpr auto comp( T&& t, U&& u )
{
    return t <=> u;
}

template<class I1, class I2>
constexpr auto lexicographical_compare_three_way(I1 f1, I1 l1, I2 f2, I2 l2)
    -> decltype(comp(*f1, *f2))
{
    bool exhaust1 = (f1 == l1);
    bool exhaust2 = (f2 == l2);
    for (; !exhaust1 && !exhaust2; exhaust1 = (++f1 == l1), exhaust2 = (++f2 == l2))
        if (auto c = comp(*f1, *f2); c != 0)
            return c;

    return !exhaust1 ? std::strong_ordering::greater:
           !exhaust2 ? std::strong_ordering::less:
                       std::strong_ordering::equal;
}

constexpr std::partial_ordering operator<=>(std::optional<float> const & lhs, std::optional<float> const & rhs)
{

    if (bool(lhs) && bool(rhs))
        return *lhs <=> *rhs;
    else if (bool(rhs))
        return std::partial_ordering::greater;
    else if (bool(lhs))
        return std::partial_ordering::less;
    else
        return std::partial_ordering::equivalent;
}

constexpr auto operator<=>(std::string_view lhs, std::string_view rhs)
{
    return lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), lhs.end());
}
#endif

constexpr auto operator<=>(std::span<uint8_t const> const & lhs, std::span<uint8_t const> const & rhs)
{
    using namespace std;
    return lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), lhs.end());
}

}
