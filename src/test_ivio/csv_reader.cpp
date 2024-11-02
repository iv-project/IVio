// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>

#include "generateSequence.h"

TEST_CASE("reading csv files", "[csv][reader]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto expected = std::vector<ivio::csv::record> {
        ivio::csv::record{.entries = {"head1", "head2"}},
        ivio::csv::record{.entries = {"value1", "value2"}},
        ivio::csv::record{.entries = {"value3", "value4"}},
    };

    auto test_data = std::string {
        "head1,head2\n"
        "value1,value2\n"
        "value3,value4\n"
    };

    SECTION("Creating Test file") {
        auto ofs = std::ofstream{tmp / "file.csv", std::ios::binary};
        ofs << test_data;
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::csv::reader{{tmp / "file.csv"}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::ifstream") {
        auto fs = std::ifstream{tmp / "file.csv"};
        auto reader = ivio::csv::reader{{fs}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::csv::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("close a reader") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::csv::reader{{ss}};
        reader.close();
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::csv::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}
