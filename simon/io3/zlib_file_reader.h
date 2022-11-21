#pragma once

#include "buffered_reader.h"
#include "file_reader.h"
#include "mmap_reader.h"
#include "stream_reader.h"

#include <zlib.h>
#include <ranges>

namespace io3 {

template <typename Reader>
struct zlib_reader_impl {
    Reader file;

    z_stream stream {
        .next_in = Z_NULL,
        .avail_in = 0,
        .total_out = 0,
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
    };

    zlib_reader_impl(Reader&& name)
        : file{std::move(name)}
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
    }
    zlib_reader_impl(zlib_reader_impl&& _other)
        : file{std::move(_other.file)}
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
    }

    ~zlib_reader_impl() {
        inflateEnd(&stream);
    }

    size_t read(std::ranges::sized_range auto&& range) {
        while(true) {
            auto [ptr, avail_in] = file.read(range.size());

            stream.next_in  = (unsigned char*)(ptr);
            stream.avail_in = avail_in;
            stream.avail_out = range.size();
            stream.next_out  = (unsigned char*)&*std::begin(range);
            auto ret = inflate(&stream, Z_NO_FLUSH);
            auto diff = avail_in - stream.avail_in;

            file.dropUntil(diff);

            auto producedBytes = (size_t)(stream.next_out - (unsigned char*)&*std::begin(range));
            if (producedBytes > 0) {
                return producedBytes;
            }
            if (ret == Z_STREAM_END) {
                return 0;
            }
        }
    }
};

using zlib_file_reader   = zlib_reader_impl<buffered_reader<file_reader>>;
using zlib_mmap_reader   = zlib_reader_impl<mmap_reader>;
using zlib_stream_reader = zlib_reader_impl<buffered_reader<stream_reader>>;

static_assert(reader_c<zlib_file_reader>);
static_assert(reader_c<zlib_mmap_reader>);
static_assert(reader_c<zlib_stream_reader>);


}
