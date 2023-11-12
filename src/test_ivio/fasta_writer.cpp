// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>

static auto read_file(std::filesystem::path p) -> std::string {
    auto s = std::filesystem::file_size(p);
    auto buffer = std::string{};
    buffer.resize(s);

    auto fs = std::ifstream{p, std::ios::binary};
    fs.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
    if (fs.gcount() != (int)buffer.size()) {
        throw std::runtime_error{"reading file failed"};
    }
    return buffer;
}


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

    auto expected = []() -> std::string {
        auto buffer = std::vector<uint8_t> {
            0x1f,0x8b,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xb3,0xf3,0x54,0x48,0xcc,0x55,
            0x48,0x4f,0xcd,0xcb,0xcf,0xcd,0x4c,0x56,0x48,0x49,0x2c,0x49,0xe4,0x72,0x74,0x76,
            0x0f,0x41,0xc6,0x5c,0x76,0xc1,0xf9,0xb9,0xa9,0x0a,0xf9,0x25,0x19,0xa9,0x45,0x10,
            0x15,0xee,0x7e,0x7e,0x7e,0xee,0x7e,0x5e,0xde,0x3e,0x5c,0x76,0xe1,0xa9,0x99,0x45,
            0x29,0x0a,0xb9,0xa5,0x39,0x25,0x99,0x39,0x99,0x79,0xa9,0x50,0x13,0xdc,0xd3,0x13,
            0xd3,0x13,0x4b,0x1c,0xdd,0x81,0x64,0xba,0xa3,0xbb,0xa3,0x97,0x37,0x50,0x83,0x1f,
            0x17,0x00,0xab,0x2e,0x16,0xcc,0x6c,0x00,0x00,0x00
        };
        auto r = std::string{};
        r.resize(buffer.size());
        memcpy(r.data(), buffer.data(), buffer.size());
        return r;
    }();

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

        CHECK(ss.str() == expected);
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

    auto expected = []() -> std::string {
        auto buffer = std::vector<uint8_t> {
            0x1f,0x8b,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x2d,0x8c,0x4b,0x0a,0xc3,0x30,
            0x0c,0x05,0xf7,0xef,0x14,0x3a,0x89,0x41,0x78,0x21,0xf2,0x41,0x9b,0x18,0xba,0x16,
            0x8d,0x70,0x0d,0x71,0x0c,0xc1,0xb9,0x7f,0x9a,0xa6,0xab,0xe1,0xc1,0xbc,0x09,0x03,
            0x59,0xa5,0xec,0x7b,0xab,0xe5,0x4d,0xab,0x75,0x03,0x47,0x41,0xe2,0x08,0x49,0x8c,
            0x28,0xe9,0xd9,0x08,0x4b,0xab,0x4e,0xad,0x7f,0xfc,0x78,0x34,0x51,0x85,0x8a,0x62,
            0x9c,0x66,0x84,0x97,0x97,0x63,0xa5,0x7a,0x6e,0xbd,0x6c,0x65,0xf7,0x7f,0x48,0x32,
            0x2c,0x1b,0x3a,0xcb,0x8f,0xf9,0x4b,0x1e,0x27,0xe8,0xfd,0xc4,0x05,0xa3,0xd4,0xfe,
            0x1c,0x7a,0x00,0x00,0x00
        };
        auto r = std::string{};
        r.resize(buffer.size());
        memcpy(r.data(), buffer.data(), buffer.size());
        return r;
    }();

    SECTION("Write to std::filesystem::path") {
        auto writer = ivio::fasta::writer{{.output = tmp / "file.fa.gz", .length = 3}};
        for (auto r : test_data) {
            writer.write(r);
        }
        writer.close();

        CHECK(read_file(tmp / "file.fa.gz") == expected);
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

        CHECK(ss.str() == expected);
    }


    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}
