#pragma once

#include "mmap_file_reader.h"

#include <zlib.h>
#include <span>
#include <vector>
#include <ranges>
#include <iostream>

struct zlib_mmap_reader {
    mmap_file_reader file;

    z_stream stream {
        .next_in = Z_NULL,
        .avail_in = 0,
        .total_out = 0,
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL,
    };

    zlib_mmap_reader(char const* name)
        : file(name)
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
        stream.avail_in = file.size();
        stream.next_in = (unsigned char*)begin(file);
    }
    zlib_mmap_reader(zlib_mmap_reader&& _other)
        : file{std::move(_other.file)}
    {
        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            throw "error";
        }
        stream.avail_in = file.size();
        stream.next_in = (unsigned char*)begin(file);

    }
    ~zlib_mmap_reader() {
        inflateEnd(&stream);
        if (file.fd == -1) return;
    }

    size_t read(std::ranges::sized_range auto&& range) {
        while(true) {
            stream.avail_out = range.size();
            stream.next_out  = (unsigned char*)&*std::begin(range);
            auto ret = inflate(&stream, Z_NO_FLUSH);
            file.doneUntil((char const*)stream.next_in);

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
