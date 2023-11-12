// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>

#include "generateSequence.h"

TEST_CASE("reading fasta files", "[fasta][reader]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto expected = std::vector<ivio::fasta::record> {
        ivio::fasta::record{.id = "I am genomic data",    .seq = "ACGTACGTACGTACGT"},
        ivio::fasta::record{.id = "Some other data",      .seq = "GNNNGNJKL"},
        ivio::fasta::record{.id = "Weird multiline data", .seq = "AGgagatAGagagAGAJKNNNN"},
    };

    auto test_data = std::string {
        ">I am genomic data\n"
        "ACGTACGTACGTACGT\n"
        ">Some other data\n"
        "GNNNGNJKL\n"
        ">Weird multiline data\n"
        "AGgagat\n"
        "AGagagAGA\n"
        "JKNNNN\n"
    };

    SECTION("Creating Test file") {
        auto ofs = std::ofstream{tmp / "file.fa", std::ios::binary};
        ofs << test_data;
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::fasta::reader{{tmp / "file.fa"}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::ifstream") {
        auto fs = std::ifstream{tmp / "file.fa"};
        auto reader = ivio::fasta::reader{{fs}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fasta::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("close a reader") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fasta::reader{{ss}};
        reader.close();
    }

    SECTION("Tell and Seek over FASTA entries over a file") {
        auto recordPositions = std::vector<size_t>{};
        auto reader = ivio::fasta::reader{{tmp / "file.fa"}};
        recordPositions.push_back(reader.tell());
        for ([[maybe_unused]] auto r : reader) {
            recordPositions.push_back(reader.tell());
        }

        for (auto p : {1, 0, 2, 0, 1, 2, 2, 2, 0, 0, 0, 1, 1, 2, 1, 0}) {
            reader.seek(recordPositions[p]);
            auto v = reader.next();
            REQUIRE(v);
            CHECK(*v == static_cast<ivio::fasta::record_view>(expected[p]));
        }
    }

    SECTION("Tell and Seek over FASTA entries over a stream") {
        auto recordPositions = std::vector<size_t>{};
        auto fs = std::ifstream{tmp / "file.fa", std::ios::binary};
        auto reader = ivio::fasta::reader{{fs}};
        recordPositions.push_back(reader.tell());
        for ([[maybe_unused]] auto r : reader) {
            recordPositions.push_back(reader.tell());
        }
        REQUIRE(recordPositions.size() == 4);

        for (auto p : {1, 0, 2, 0, 1, 2, 2, 2, 0, 0, 0, 1, 1, 2, 1, 0}) {
            reader.seek(recordPositions[p]);
            auto v = reader.next();
            REQUIRE(v);
            CHECK(*v == static_cast<ivio::fasta::record_view>(expected[p]));
        }
    }

    SECTION("Tell and Seek over FASAT entries, compatible with faidx") {
        auto recordPositions = std::vector<ivio::faidx::record>{};
        auto reader = ivio::fasta::reader{{tmp / "file.fa"}};
        for ([[maybe_unused]] auto r : reader) {
            recordPositions.push_back(reader.tell_faidx());
        }
        REQUIRE(recordPositions.size() == 3);

        for (auto p : {1, 0, 2, 0, 1, 2, 2, 2, 0, 0, 0, 1, 1, 2, 1, 0}) {
            reader.seek_faidx(recordPositions[p]);
            auto v = reader.next();
            REQUIRE(v);
            CHECK(*v == static_cast<ivio::fasta::record_view>(expected[p]));
        }
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fasta::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}

TEST_CASE("reading large fasta files", "[fasta][reader][large]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    // Generate a file with very much data
    auto expected = std::vector<ivio::fasta::record>{};

    auto test_data = std::string{};
    srand(0);
    for (size_t i{0}; i < 1024; ++i) {
        expected.push_back({
            .id  = "sequence id " + std::to_string(i),
            .seq = generateSequence(250)
        });
        test_data += ">" + expected.back().id + "\n";
        test_data += expected.back().seq + "\n";
    }

    SECTION("Creating Test file") {
        auto ofs = std::ofstream{tmp / "file.fa", std::ios::binary};
        ofs << test_data;
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::fasta::reader{{tmp / "file.fa"}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::ifstream") {
        auto fs = std::ifstream{tmp / "file.fa"};
        auto reader = ivio::fasta::reader{{fs}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fasta::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("close a reader") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fasta::reader{{ss}};
        reader.close();
    }

    SECTION("Tell and Seek over FASTA entries over a file") {
        auto recordPositions = std::vector<size_t>{};
        auto reader = ivio::fasta::reader{{tmp / "file.fa"}};
        recordPositions.push_back(reader.tell());
        for ([[maybe_unused]] auto r : reader) {
            recordPositions.push_back(reader.tell());
        }

        srand(0);
        for (size_t i{0}; i < 10'000; ++i) {
            auto p = rand() % 1024;
            reader.seek(recordPositions[p]);
            auto v = reader.next();
            REQUIRE(v);
            CHECK(*v == static_cast<ivio::fasta::record_view>(expected[p]));
        }
    }

    SECTION("Tell and Seek over FASTA entries over a stream") {
        auto recordPositions = std::vector<size_t>{};
        auto fs = std::ifstream{tmp / "file.fa", std::ios::binary};
        auto reader = ivio::fasta::reader{{fs}};
        recordPositions.push_back(reader.tell());
        for ([[maybe_unused]] auto r : reader) {
            recordPositions.push_back(reader.tell());
        }
        REQUIRE(recordPositions.size() == 1025);

        srand(0);
        for (size_t i{0}; i < 10'000; ++i) {
            auto p = rand() % 1024;
            reader.seek(recordPositions[p]);
            auto v = reader.next();
            REQUIRE(v);
            CHECK(*v == static_cast<ivio::fasta::record_view>(expected[p]));
        }
    }

    SECTION("Tell and Seek over FASAT entries, compatible with faidx") {
        auto recordPositions = std::vector<ivio::faidx::record>{};
        auto reader = ivio::fasta::reader{{tmp / "file.fa"}};
        for ([[maybe_unused]] auto r : reader) {
            recordPositions.push_back(reader.tell_faidx());
        }
        REQUIRE(recordPositions.size() == 1024);

        srand(0);
        for (size_t i{0}; i < 10'000; ++i) {
            auto p = rand() % 1024;
            reader.seek_faidx(recordPositions[p]);
            auto v = reader.next();
            REQUIRE(v);
            CHECK(*v == static_cast<ivio::fasta::record_view>(expected[p]));
        }

        auto invalid_faidx_record = ivio::faidx::record {
            .id        = "some id",
            .length    = 10,
            .offset    = 2,
            .linebases = 80,
            .linewidth = 81,
        };
        CHECK_THROWS(reader.seek_faidx(invalid_faidx_record));
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fasta::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}

TEST_CASE("reading large fasta files with lfcr line ending", "[fasta][reader][large][lfcr]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    // Generate a file with very much data
    auto expected = std::vector<ivio::fasta::record>{};

    auto test_data = std::string{};
    srand(0);
    for (size_t i{0}; i < 1024; ++i) {
        expected.push_back({
            .id  = "sequence id " + std::to_string(i),
            .seq = generateSequence(250)
        });
        test_data += ">" + expected.back().id + "\r\n";
        test_data += expected.back().seq + "\r\n";
    }

    SECTION("Creating Test file") {
        auto ofs = std::ofstream{tmp / "file.fa", std::ios::binary};
        ofs << test_data;
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::fasta::reader{{tmp / "file.fa"}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::ifstream") {
        auto fs = std::ifstream{tmp / "file.fa"};
        auto reader = ivio::fasta::reader{{fs}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fasta::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("close a reader") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fasta::reader{{ss}};
        reader.close();
    }

    SECTION("Tell and Seek over FASTA entries over a file") {
        auto recordPositions = std::vector<size_t>{};
        auto reader = ivio::fasta::reader{{tmp / "file.fa"}};
        recordPositions.push_back(reader.tell());
        for ([[maybe_unused]] auto r : reader) {
            recordPositions.push_back(reader.tell());
        }

        srand(0);
        for (size_t i{0}; i < 10'000; ++i) {
            auto p = rand() % 1024;
            reader.seek(recordPositions[p]);
            auto v = reader.next();
            REQUIRE(v);
            CHECK(*v == static_cast<ivio::fasta::record_view>(expected[p]));
        }
    }

    SECTION("Tell and Seek over FASTA entries over a stream") {
        auto recordPositions = std::vector<size_t>{};
        auto fs = std::ifstream{tmp / "file.fa", std::ios::binary};
        auto reader = ivio::fasta::reader{{fs}};
        recordPositions.push_back(reader.tell());
        for ([[maybe_unused]] auto r : reader) {
            recordPositions.push_back(reader.tell());
        }
        REQUIRE(recordPositions.size() == 1025);

        srand(0);
        for (size_t i{0}; i < 10'000; ++i) {
            auto p = rand() % 1024;
            reader.seek(recordPositions[p]);
            auto v = reader.next();
            REQUIRE(v);
            CHECK(*v == static_cast<ivio::fasta::record_view>(expected[p]));
        }
    }

    SECTION("Tell and Seek over FASAT entries, compatible with faidx") {
        auto recordPositions = std::vector<ivio::faidx::record>{};
        auto reader = ivio::fasta::reader{{tmp / "file.fa"}};
        for ([[maybe_unused]] auto r : reader) {
            recordPositions.push_back(reader.tell_faidx());
        }
        REQUIRE(recordPositions.size() == 1024);

        srand(0);
        for (size_t i{0}; i < 10'000; ++i) {
            auto p = rand() % 1024;
            reader.seek_faidx(recordPositions[p]);
            auto v = reader.next();
            REQUIRE(v);
            CHECK(*v == static_cast<ivio::fasta::record_view>(expected[p]));
        }

        auto invalid_faidx_record = ivio::faidx::record {
            .id        = "some id",
            .length    = 10,
            .offset    = 2,
            .linebases = 80,
            .linewidth = 81,
        };
        CHECK_THROWS(reader.seek_faidx(invalid_faidx_record));

        auto invalid_faidx_record_lineending = ivio::faidx::record {
            .id        = "some id",
            .length    = 10,
            .offset    = 2,
            .linebases = 80,
            .linewidth = 83,
        };
        CHECK_THROWS(reader.seek_faidx(invalid_faidx_record_lineending));

    }

    SECTION("Read from std::stringstream") {
        auto ss = std::stringstream{test_data};
        auto reader = ivio::fasta::reader{{ss}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}

TEST_CASE("reading compressed fasta files", "[fasta][reader][gz][short]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto test_data = []() -> std::string {
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


    auto expected = std::vector<ivio::fasta::record> {
        ivio::fasta::record{.id = "I am genomic data",    .seq = "ACGTACGTACGTACGT"},
        ivio::fasta::record{.id = "Some other data",      .seq = "GNNNGNJKL"},
        ivio::fasta::record{.id = "Weird multiline data", .seq = "AGgagatAGagagAGAJKNNNN"},
    };

    SECTION("prepare, write the test data") {
        auto ofs = std::ofstream{tmp / "file.fa.gz", std::ios::binary};
        ofs.write(test_data.data(), test_data.size());
        ofs.close();
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::fasta::reader{{tmp / "file.fa.gz"}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }

    SECTION("Read from std::ifstream") {
        auto fs = std::ifstream{tmp / "file.fa.gz"};
        auto reader = ivio::fasta::reader{{.input = fs, .compressed = true}};
        auto vec = std::vector(begin(reader), end(reader));
        static_assert(std::same_as<decltype(vec), decltype(expected)>, "vec and expected should have the exact same type");
        CHECK(expected == vec);
    }


    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}
