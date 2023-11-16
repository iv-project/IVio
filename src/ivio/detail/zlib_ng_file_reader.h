// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "buffered_reader.h"
#include "file_reader.h"
#include "mmap_reader.h"
#include "stream_reader.h"

#include "zlib-ng.h"
#include <ranges>

namespace ivio {

struct zlib_ng_reader {
    VarBufferedReader reader;

    zng_stream stream = []() {
        auto stream = zng_stream{};
        stream.next_in   = NULL;
        stream.avail_in  = 0;
        stream.total_out = 0;
        stream.zalloc    = NULL;
        stream.zfree     = NULL;
        stream.opaque    = NULL;
        return stream;
    }();

    zlib_ng_reader(VarBufferedReader reader)
        : reader{std::move(reader)}
    {
        if (zng_inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
    }
    zlib_ng_reader(zlib_ng_reader&& _other)
        : reader{std::move(_other.reader)}
    {
        if (zng_inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
    }

    ~zlib_ng_reader() {
        zng_inflateEnd(&stream);
    }

    size_t read(std::ranges::contiguous_range auto&& range) {
        static_assert(std::same_as<std::ranges::range_value_t<decltype(range)>, char>);
        while(true) {
            auto [ptr, avail_in] = reader.read(range.size());

            stream.next_in  = (unsigned char*)(ptr);
            stream.avail_in = avail_in;
            stream.avail_out = range.size();
            stream.next_out  = (unsigned char*)range.data();
            auto ret = zng_inflate(&stream, Z_NO_FLUSH);
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

static_assert(Readable<zlib_ng_reader>);
}
