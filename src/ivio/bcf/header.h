// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <vector>
#include <string>

namespace ivio::bcf {

struct header {
    std::vector<std::tuple<std::string, std::string>> table;
    std::vector<std::string> genotypes;
};

}
