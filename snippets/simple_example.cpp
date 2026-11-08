// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: CC0-1.0
#include <ivio/ivio.h>

int main() {
    auto reader = ivio::fasta::reader{{"somedata.fasta"}};
    for (auto record_view : reader) {
        std::cout << "id: " << record_view.id << "\n";
        std::cout << "seq: " << record_view.seq << "\n";
    }
}
