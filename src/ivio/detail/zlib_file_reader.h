// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "buffered_reader.h"
#include "file_reader.h"
#include "mmap_reader.h"
#include "stream_reader.h"

#include <zlib.h>
#include <ranges>

namespace ivio {

struct zlib_reader {
    VarBufferedReader reader;

    z_stream stream = []() {
        auto stream = z_stream{};
        stream.next_in   = Z_NULL;
        stream.avail_in  = 0;
        stream.total_out = 0;
        stream.zalloc    = Z_NULL;
        stream.zfree     = Z_NULL;
        stream.opaque    = Z_NULL;
        return stream;
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

            stream.next_in  = (unsigned char*)(ptr);
            stream.avail_in = avail_in;
            stream.avail_out = range.size();
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
};

static_assert(Readable<zlib_reader>);

}
