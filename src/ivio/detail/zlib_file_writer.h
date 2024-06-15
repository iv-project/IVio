// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "buffered_writer.h"
#include "file_writer.h"
#include "stream_writer.h"

#include <array>
#include <cassert>
#include <ranges>
#include <zlib.h>

namespace ivio {

template <writer_c writer>
struct zlib_writer_impl {
    writer file;

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

    zlib_writer_impl() = delete;
    zlib_writer_impl(writer&& name)
        : file{std::move(name)}
    {
        if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            throw std::runtime_error{"error initializing zlib/deflateInit2"};
        }
    }

    zlib_writer_impl(zlib_writer_impl const& _other) = delete;
    zlib_writer_impl(zlib_writer_impl&& _other)
        : file{std::move(_other.file)}
    {
        if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            throw std::runtime_error{"error initializing zlib/deflateInit2"};
        }
    }

    ~zlib_writer_impl() {
        close();
    }

    auto operator=(zlib_writer_impl&&) = delete;
    auto operator=(zlib_writer_impl const&) = delete;

    auto write(std::span<char> buffer) -> size_t {
        assert(buffer.size() <= std::numeric_limits<uint32_t>::max());
        auto outBuffer = std::array<char, 2<<16>{};

        stream.next_in = (unsigned char*)buffer.data();
        stream.avail_in = static_cast<uint32_t>(buffer.size());
        stream.next_out  = (unsigned char*)&outBuffer[0];
        stream.avail_out = static_cast<uint32_t>(outBuffer.size());

        auto ret = deflate(&stream, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            throw std::runtime_error{"error deflating data with zlib"};
        }
        file.write({&outBuffer[0], outBuffer.size() - stream.avail_out});
        return buffer.size() - stream.avail_in;
    }

    void close() {
        if (stream.next_in) {
            auto outBuffer = std::array<char, 2<<16>{};

            do {
                stream.next_out  = (unsigned char*)&outBuffer[0];
                stream.avail_out = static_cast<uint32_t>(outBuffer.size());

                auto ret = deflate(&stream, Z_FINISH);
                if (ret != Z_OK && ret != Z_STREAM_END) {
                    throw std::runtime_error{"error deflating data with zlib"};
                }
                file.write({&outBuffer[0], outBuffer.size() - stream.avail_out});
            } while (stream.avail_in);
            file.close();
            stream.next_in = nullptr;
        }
        deflateEnd(&stream);
    }
};

using zlib_file_writer   = zlib_writer_impl<file_writer>;
using zlib_stream_writer = zlib_writer_impl<stream_writer>;

static_assert(writer_c<zlib_file_writer>);
static_assert(writer_c<zlib_stream_writer>);

}
