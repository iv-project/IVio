// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: CC0-1.0
#include <ivio/ivio.h>
#include <iostream>

int main(int argc, char** argv) {
    auto file = std::filesystem::path{argv[1]};
    for (auto view : ivio::vcf::reader{{file}}) {
        std::cout << view.ref << "\n";
    }
}
