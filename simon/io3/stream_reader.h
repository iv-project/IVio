#pragma once

#include "concepts.h"

#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <array>
#include <string>
#include <istream>

namespace io3 {

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
        stream.read(&*std::ranges::begin(range), std::ranges::size(range));
        return stream.gcount();
    }
};

static_assert(reader_c<stream_reader>);

}
