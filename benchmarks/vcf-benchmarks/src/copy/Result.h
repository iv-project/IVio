// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file.
// -----------------------------------------------------------------------------------------------------
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
