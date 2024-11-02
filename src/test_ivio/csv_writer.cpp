// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include "utilities.h"

#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>
#include <zlib.h>

TEST_CASE("writing csv files", "[csv][writer]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto test_data = std::vector<ivio::csv::record> {
        ivio::csv::record{.entries = {"head1", "head2"}},
        ivio::csv::record{.entries = {"value1", "value2"}},
        ivio::csv::record{.entries = {"value3", "value4"}},
    };

    auto expected = std::string {
        "head1,head2\n"
        "value1,value2\n"
        "value3,value4\n"
    };

    SECTION("Write to std::filesystem::path") {
        auto writer = ivio::csv::writer{{tmp / "file.csv"}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();
        CHECK(read_file(tmp / "file.csv") == expected);
    }

    SECTION("Write to std::ofstream") {
        auto fs = std::ofstream{tmp / "file.csv", std::ios::binary};
        auto writer = ivio::csv::writer{{fs}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();
        fs.close();
        CHECK(read_file(tmp / "file.csv") == expected);
    }

    SECTION("Write to std::stringstream") {
        auto ss = std::stringstream{};
        auto writer = ivio::csv::writer{{ss}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();
        CHECK(ss.str() == expected);
    }

    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}
