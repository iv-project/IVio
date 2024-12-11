// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "concepts.h"

#include <istream>
#include <limits>
#include <ranges>

namespace ivio {

class stream_reader {
protected:
    std::istream& stream;

public:
    stream_reader(std::istream& _stream)
        : stream{_stream}
    {}

    stream_reader() = delete;
    stream_reader(stream_reader const&) = delete;
    stream_reader(stream_reader&& _other) noexcept = default;
    ~stream_reader() = default;

    auto operator=(stream_reader const&) -> stream_reader& = delete;
    auto operator=(stream_reader&&) -> stream_reader& = delete;

    size_t read(std::ranges::contiguous_range auto&& range) const {
        static_assert(std::same_as<std::ranges::range_value_t<decltype(range)>, char>);
        return stream.rdbuf()->sgetn(std::ranges::data(range), std::ranges::size(range));
    }

    // like read, but does not advance internal pointers
    size_t peek(std::ranges::contiguous_range auto&& range) const {
        size_t pos = tell();
        auto len = read(range);
        if (pos != std::numeric_limits<size_t>::max()) { // check if seeking is supported
            stream.seekg(pos);
        } else { // put the stuff back!
            for (size_t i{0}; i < len; ++i) {
                stream.putback(range[len-i-1]);
            }
        }
        return len;
    }

    auto tell() const -> size_t {
        return stream.tellg();
    }

    void seek(size_t offset) {
        stream.seekg(offset);
    }
};

static_assert(Readable<stream_reader>);
static_assert(Seekable<stream_reader>);
}
