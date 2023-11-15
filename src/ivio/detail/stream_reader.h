// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "concepts.h"

#include <istream>
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

    size_t read(std::ranges::sized_range auto&& range) const {
        return stream.rdbuf()->sgetn(std::ranges::data(range), std::ranges::size(range));
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
