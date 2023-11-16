// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <algorithm>
#include <cstdint>
#include <span>

namespace ivio {

constexpr auto operator<=>(std::span<uint8_t const> const & lhs, std::span<uint8_t const> const & rhs)
{
    return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), lhs.end());
}

}
