// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>

#include "generateSequence.h"

TEST_CASE("reading faidx files", "[faidx][reader]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto expected = std::vector<ivio::faidx::record> {
        ivio::faidx::record{.id = "I-am-genomic-data",    .length = 16, .offset = 19, .linebases = 3, .linewidth = 4},
        ivio::faidx::record{.id = "Some-other-data",      .length = 9,  .offset = 58, .linebases = 3, .linewidth = 4},
        ivio::faidx::record{.id = "Weird-multiline-data", .length = 22, .offset = 92, .linebases = 3, .linewidth = 4},
    };

    auto const test_data = std::string {
        "I-am-genomic-data	16	19	3	4\n"
        "Some-other-data	9	58	3	4\n"
        "Weird-multiline-data	22	92	3	4\n"
    };

    SECTION("Creating Test file") {
        auto ofs = std::ofstream{tmp / "file.fa.faidx", std::ios::binary};
        ofs << test_data;
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::faidx::reader{{tmp / "file.fa.faidx"}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::ifstream") {
        auto fs = std::ifstream{tmp / "file.fa.faidx"};
        auto reader = ivio::faidx::reader{{fs}};
//        auto reader = ivio::faidx::reader{{tmp / "file.fa.faidx"}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

/*    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::faidx::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }*/

    SECTION("close a reader") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::faidx::reader{{ss}};
        reader.close();
    }
}
