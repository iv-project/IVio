// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: CC0-1.0

#include <iostream>
#include <ivio/ivio.h>

int main(int, char**) {
    auto reader = ivio::fasta::reader{{.input = std::cin,
                                       .compressed = false, // false by default, if true a gzip file is expected
    }};

    // creates a vector of type `std::vector<ivio::fasta::record>` even though reader returns `record_view`
    auto vec = std::vector{begin(reader), end(reader)};
}

