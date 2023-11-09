// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: CC0-1.0
#include <iostream>
#include <ivio/ivio.h>

int main(int, char**) {
    auto reader = ivio::fasta::reader{{.input = std::cin}};
    auto writer = ivio::fasta::writer{{.input = std::cout}};

    for (auto record_view : reader) {
        writer.write(record_view);
    }
}

