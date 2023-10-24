// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file.
// -----------------------------------------------------------------------------------------------------
#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>
#include <ivio/ivio.h>
#include <ivio/file_reader.h>

TEST_CASE("reading some file", "[file][reader]") {
    auto tmp = std::filesystem::temp_directory_path() / "ivio_test";
    std::filesystem::create_directory(tmp);

    auto test_data = std::string {
        "I am a file with some lines\n" // 28 chars
        "This is my second line\n"      // 23 chars
        "A third line\n"                // 15 chars
    };

    SECTION("Creating Test file") {
        auto ofs = std::ofstream{tmp / "file.txt", std::ios::binary};
        ofs << test_data;
    }

    SECTION("Read from std::filesystem::path") {
        auto reader = ivio::file_reader{tmp / "file.txt"};
        auto buffer = std::vector<char>{};
        buffer.resize(test_data.size());

        auto size = reader.read(buffer); // read complete file at once
        REQUIRE(size == test_data.size());
        EXPECT(std::span{buffer} == std::span{test_data});
    }

    SECTION("cleanup - deleting temp folder") {
        std::filesystem::remove_all(tmp);
    }
}
