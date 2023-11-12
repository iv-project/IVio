// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cstdlib>
#include <string>

inline auto generateSequence(size_t size) -> std::string {
    auto s = std::string{};
    s.reserve(size);
    while (s.size() < size) {
        s = s + static_cast<char>(rand() % 26 + 'a');
    }
    return s;
}
