// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "concepts.h"

#include <algorithm>
#include <span>
#include <vector>

namespace ivio {

template <writer_c Writer, size_t minV = 2<<16>
class buffered_writer {
    Writer writer;

    std::vector<char> buf = []() { auto vec = std::vector<char>{}; vec.reserve(minV); return vec; }();
    int inPos{};

    std::vector<char> buffer;

public:
    buffered_writer(Writer&& _other)
        : writer{std::move(_other)}
    {}
    buffered_writer(buffered_writer const&) = delete;
    buffered_writer(buffered_writer&& _other) = default;
    ~buffered_writer() {
        close();
    }

    auto write(std::span<char const> _buffer) -> size_t {
        auto oldSize = buffer.size();
        buffer.resize(oldSize + _buffer.size());
//!WORKAROUND llvm < 16 does not provide std::ranges::copy
#if defined(_LIBCPP_VERSION) && _LIBCPP_VERSION < 160000
        std::copy(_buffer.begin(), _buffer.end(), buffer.begin()+oldSize);
#else
        std::ranges::copy(_buffer, buffer.begin()+oldSize);
#endif
        if (buffer.size() > minV) {
            auto writtenBytes = writer.write(buffer);
            if (writtenBytes > 0) {
                std::copy(buffer.begin()+writtenBytes, buffer.end(), buffer.begin());
                buffer.resize(buffer.size() - writtenBytes);
            }
        }
        return _buffer.size();
    }

    void close() {
        while (buffer.size()) {
            auto writtenBytes = writer.write(buffer);
            if (writtenBytes > 0) {
                std::copy(buffer.begin()+writtenBytes, buffer.end(), buffer.begin());
                buffer.resize(buffer.size() - writtenBytes);
            } else {
                buffer.clear();
            }
        }
        writer.close();
    }

};

//static_assert(writer_and_dropper_c<buffered_writer<file_writer>>);
}
