// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include "utilities.h"

#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>
#include <zlib.h>

TEST_CASE("writing fasta files", "[fasta][writer]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto test_data = std::vector<ivio::fasta::record> {
        ivio::fasta::record{.id = "I am genomic data",    .seq = "ACGTACGTACGTACGT"},
        ivio::fasta::record{.id = "Some other data",      .seq = "GNNNGNJKL"},
        ivio::fasta::record{.id = "Weird multiline data", .seq = "AGgagatAGagagAGAJKNNNN"},
    };

    auto expected = std::string {
        ">I am genomic data\n"
        "ACGTACGTACGTACGT\n"
        ">Some other data\n"
        "GNNNGNJKL\n"
        ">Weird multiline data\n"
        "AGgagatAGagagAGAJKNNNN\n"
    };

    SECTION("Write to std::filesystem::path") {
        auto writer = ivio::fasta::writer{{tmp / "file.fa"}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();
        CHECK(read_file(tmp / "file.fa") == expected);
    }

    SECTION("Write to std::ofstream") {
        auto fs = std::ofstream{tmp / "file.fa"};
        auto writer = ivio::fasta::writer{{fs}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();
        fs.close();
        CHECK(read_file(tmp / "file.fa") == expected);
    }

    SECTION("Write to std::stringstream") {
        auto ss = std::stringstream{};
        auto writer = ivio::fasta::writer{{ss}};
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

TEST_CASE("writing fasta files (compressed)", "[fasta][writer][gz]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto test_data = std::vector<ivio::fasta::record> {
        ivio::fasta::record{.id = "I am genomic data",    .seq = "ACGTACGTACGTACGT"},
        ivio::fasta::record{.id = "Some other data",      .seq = "GNNNGNJKL"},
        ivio::fasta::record{.id = "Weird multiline data", .seq = "AGgagatAGagagAGAJKNNNN"},
    };

    auto expected = std::string {
        ">I am genomic data\n"
        "ACGTACGTACGTACGT\n"
        ">Some other data\n"
        "GNNNGNJKL\n"
        ">Weird multiline data\n"
        "AGgagatAGagagAGAJKNNNN\n"
    };

    SECTION("Write to std::filesystem::path") {
        auto writer = ivio::fasta::writer{{tmp / "file.fa.gz"}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();

        CHECK(read_file(tmp / "file.fa.gz") == expected);
    }

    SECTION("Write to std::ofstream") {
        auto fs = std::ofstream{tmp / "file.fa.gz"};
        auto writer = ivio::fasta::writer{{.output = fs, .compressed = true}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();

        CHECK(read_file(tmp / "file.fa.gz") == expected);
    }

    SECTION("Write to std::stringstream") {
        auto ss = std::stringstream{};
        auto writer = ivio::fasta::writer{{.output = ss, .compressed = true}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();

        CHECK(read_compressed_string(ss.str(), tmp / "tmp.fasta.gz" ) == expected);
    }


    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}

TEST_CASE("writing fasta files, short lines (compressed)", "[fasta][writer][gz][short]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto test_data = std::vector<ivio::fasta::record> {
        ivio::fasta::record{.id = "I am genomic data",    .seq = "ACGTACGTACGTACGT"},
        ivio::fasta::record{.id = "Some other data",      .seq = "GNNNGNJKL"},
        ivio::fasta::record{.id = "Weird multiline data", .seq = "AGgagatAGagagAGAJKNNNN"},
    };

    auto expected = std::string {
        ">I am genomic data\n"
        "ACG\n"
        "TAC\n"
        "GTA\n"
        "CGT\n"
        "ACG\n"
        "T\n"
        ">Some other data\n"
        "GNN\n"
        "NGN\n"
        "JKL\n"
        ">Weird multiline data\n"
        "AGg\n"
        "aga\n"
        "tAG\n"
        "aga\n"
        "gAG\n"
        "AJK\n"
        "NNN\n"
        "N\n"
    };

    SECTION("Write to std::filesystem::path") {
        auto writer = ivio::fasta::writer{{.output = tmp / "file.fa.gz", .length = 3}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();

        auto xx = read_file(tmp / "file.fa.gz");
        CHECK(xx.size() == expected.size());


        CHECK(xx == expected);
    }

    SECTION("Write to std::ofstream") {
        auto fs = std::ofstream{tmp / "file.fa.gz"};
        auto writer = ivio::fasta::writer{{.output = fs, .compressed = true, .length = 3}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();

        CHECK(read_file(tmp / "file.fa.gz") == expected);
    }

    SECTION("Write to std::stringstream") {
        auto ss = std::stringstream{};
        auto writer = ivio::fasta::writer{{.output = ss, .compressed = true, .length = 3}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();

        CHECK(read_compressed_string(ss.str(), tmp / "tmp.fasta.gz" ) == expected);
    }


    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}
