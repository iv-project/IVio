// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "buffered_reader.h"
#include "file_reader.h"
#include "mmap_reader.h"
#include "stream_reader.h"

#include <fstream>
#include <ranges>
#include <zlib.h>

namespace ivio {

struct zlib_reader {
    VarBufferedReader reader;

    z_stream stream = []() {
        auto _stream = z_stream{};
        _stream.next_in   = Z_NULL;
        _stream.avail_in  = 0;
        _stream.total_out = 0;
        _stream.zalloc    = Z_NULL;
        _stream.zfree     = Z_NULL;
        _stream.opaque    = Z_NULL;
        return _stream;
    }();

    zlib_reader(VarBufferedReader reader)
        : reader{std::move(reader)}
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw std::runtime_error{"error"};
        }
    }
    zlib_reader(zlib_reader&& _other)
        : reader{std::move(_other.reader)}
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw std::runtime_error{"error"};
        }
    }

    ~zlib_reader() {
        inflateEnd(&stream);
    }

    size_t read(std::ranges::contiguous_range auto&& range) {
        static_assert(std::same_as<std::ranges::range_value_t<decltype(range)>, char>);
        while(true) {
            auto [ptr, avail_in] = reader.read(range.size());
            avail_in = std::min<size_t>(std::numeric_limits<uint32_t>::max(), avail_in);
            auto avail_out = std::min<size_t>(std::numeric_limits<uint32_t>::max(), range.size());

            stream.next_in  = (unsigned char*)(ptr);
            stream.avail_in = static_cast<uint32_t>(avail_in);
            stream.avail_out = static_cast<uint32_t>(avail_out);
            stream.next_out  = (unsigned char*)range.data();
            auto ret = inflate(&stream, Z_NO_FLUSH);
            auto diff = avail_in - stream.avail_in;

            reader.dropUntil(diff);

            auto producedBytes = (size_t)(stream.next_out - (unsigned char*)range.data());
            if (producedBytes > 0) {
                return producedBytes;
            }
            if (ret == Z_STREAM_END) {
                return 0;
            }
        }
    }

    /**
     * Function checks if the given buffer contains the magic header of a gz file.
     * See https://www.ietf.org/rfc/rfc1952.txt
     */
    static bool isGZipHeader(std::span<char const> buffer) {
        auto view = std::string_view{buffer.data(), buffer.size()};
        return view.starts_with("\x1f\x8b");
    }
};

static_assert(Readable<zlib_reader>);

inline auto makeZlibReader(std::filesystem::path file) -> VarBufferedReader {
    if (is_regular_file(file)) {
        auto reader = mmap_reader{file}; // create a reader and peak into the file
        auto [buffer, len] = reader.read(2);
        if (zlib_reader::isGZipHeader({buffer, len})) {
            return zlib_reader{std::move(reader)};
        }
        return reader;
    } else {
        auto ifs = std::make_shared<std::ifstream>(file, std::ios::binary);

        auto reader = stream_reader{*ifs}; // create a reader and peak into the file
        auto buffer = std::array<char, 2>{};
        auto len = reader.peek(buffer);
        if (zlib_reader::isGZipHeader({buffer.data(), len})) {
            return zlib_reader{std::move(reader)};
        }
        return {std::move(reader), std::any{ifs}};
    }
}
inline auto makeZlibReader(std::istream& file) -> VarBufferedReader {
    auto reader = stream_reader{file};
    auto buffer = std::array<char, 2>{};
    auto len = reader.peek(buffer);
    if (zlib_reader::isGZipHeader({buffer.data(), len})) {
        return zlib_reader{std::move(reader)};
    }
    return reader;
}

}
