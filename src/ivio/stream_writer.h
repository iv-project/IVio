#pragma once

#include "concepts.h"

#include <filesystem>
#include <ostream>

namespace ivio {

class stream_writer {
protected:
    std::ostream& ofs;

public:
    stream_writer(std::ostream& ofs)
        : ofs{ofs}
    {}

    stream_writer() = delete;
    stream_writer(stream_writer const&) = delete;
    stream_writer(stream_writer&& _other) noexcept = default;
    ~stream_writer() = default;

    auto operator=(stream_writer const&) -> stream_writer& = delete;
    auto operator=(stream_writer&&) -> stream_writer& = delete;

    auto write(std::span<char const> buffer) -> size_t {
        ofs.write(buffer.data(), buffer.size());
        return buffer.size();
    }

    void close() {
        ofs << std::flush;
    }
};

static_assert(writer_c<stream_writer>);
}
