// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>

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

static auto generateSequence(size_t size) -> std::string {
    auto s = std::string{};
    s.reserve(size);
    while (s.size() < size) {
        s = s + static_cast<char>(rand() % 26 + 'a');
    }
    return s;
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
