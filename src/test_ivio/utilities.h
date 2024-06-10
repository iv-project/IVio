// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <zlib.h>

/**
 * Reads the file into a single string. If file is gzip compressed, decompressing it
 */
inline auto read_file(std::filesystem::path p) -> std::string {
    auto buffer = std::string{};

    auto f = gzopen(p.string().c_str(), "r");
    if(f == Z_NULL) throw std::runtime_error{"error opening " + p.string()};

    int bytes_read = 0;
    do {
        auto oldSize = buffer.size();
        buffer.resize(oldSize + 65535);
        assert(buffer.size() <= std::numeric_limits<uint32_t>::max());
        bytes_read = gzread(f, buffer.data(), static_cast<uint32_t>(buffer.size()));
        if (bytes_read < 0) {
            throw std::runtime_error{"reading file failed"};
        }
        buffer.resize(oldSize + bytes_read);
    } while (bytes_read != 0);
    gzclose(f);
    return buffer;
}

/**
 * Decompresses the string `buffer` into a string by writing it into a temporary file and
 * reading that.
 */
inline auto read_compressed_string(std::string buffer, std::filesystem::path tmpFile) -> std::string {
    auto ofs = std::ofstream{tmpFile, std::ios::binary};
    ofs.write(buffer.c_str(), buffer.size());
    ofs.close();
    return read_file(tmpFile);
}
