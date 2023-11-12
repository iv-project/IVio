// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>

#include "generateSequence.h"

TEST_CASE("reading fastq files", "[fastq][reader]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto expected = std::vector<ivio::fastq::record> {
        ivio::fastq::record{.id = "I am genomic data",.seq = "ACGTACGTACGTACGT",                                  .id2 = "", .qual = "!!!!!!!!!!!!!!!!"},
        ivio::fastq::record{.id = "Some other data",  .seq = "GNNNGNJKL",                                         .id2 = "", .qual = "!!!!!!!!!" },
        ivio::fastq::record{.id = "long data",        .seq = "AGgagatAGagagAGAJKNNNNuiaeruniaetHallokeineahnung", .id2 = "", .qual = "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"},
    };

    auto test_data = std::string {
        "@I am genomic data\n"
        "ACGTACGTACGTACGT\n"
        "+\n"
        "!!!!!!!!!!!!!!!!\n"
        "@Some other data\n"
        "GNNNGNJKL\n"
        "+\n"
        "!!!!!!!!!\n"
        "@long data\n"
        "AGgagatAGagagAGAJKNNNNuiaeruniaetHallokeineahnung\n"
        "+\n"
        "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
    };

    SECTION("Creating Test file") {
        auto ofs = std::ofstream{tmp / "file.fq", std::ios::binary};
        ofs << test_data;
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::fastq::reader{{tmp / "file.fq"}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::ifstream") {
        auto fs = std::ifstream{tmp / "file.fq"};
        auto reader = ivio::fastq::reader{{fs}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fastq::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("close a reader") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fastq::reader{{ss}};
        reader.close();
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fastq::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}

TEST_CASE("reading fastq files over large files", "[fastq][reader][large]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto expected = std::vector<ivio::fastq::record>{};

    auto test_data = std::string{};
    srand(0);
    for (size_t i{0}; i < 1024; ++i) {
        expected.push_back({
            .id   = "sequence id " + std::to_string(i),
            .seq  = generateSequence(250),
            .id2  = "",
            .qual = std::string(250, '!'),
        });
        test_data += "@" + expected.back().id + "\n";
        test_data += expected.back().seq + "\n";
        test_data += "+" + expected.back().id2 + "\n";
        test_data += expected.back().qual + "\n";
    }

    SECTION("Creating Test file") {
        auto ofs = std::ofstream{tmp / "file.fq", std::ios::binary};
        ofs << test_data;
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::fastq::reader{{tmp / "file.fq"}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::ifstream") {
        auto fs = std::ifstream{tmp / "file.fq"};
        auto reader = ivio::fastq::reader{{fs}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fastq::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("close a reader") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fastq::reader{{ss}};
        reader.close();
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fastq::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}
