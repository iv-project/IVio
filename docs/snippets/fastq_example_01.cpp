// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: CC0-1.0
#include <iostream>
#include <ivio/ivio.h>

int main(int /*argc*/, char** argv) {
    auto inputFile = std::filesystem::path{argv[1]};
    auto reader = ivio::fastq::reader{{.input = inputFile,
                                       .compressed = false, // false by default, if true a gzip file is expected
    }};
    for (auto record_view : reader) {
        std::cout << "id: "   << record_view.id << "\n";
        std::cout << "seq: "  << record_view.seq << "\n";
        std::cout << "id2: "  << record_view.id2 << "\n";
        std::cout << "qual: " << record_view.qual << "\n";

    }
}

