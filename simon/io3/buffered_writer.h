#pragma once

#include "concepts.h"

#include <span>
#include <vector>

namespace io3 {

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


    void write(std::span<char> _buffer, bool finish) {
        auto oldSize = buffer.size();
        buffer.resize(oldSize + _buffer.size());
        std::ranges::copy(_buffer, buffer.begin()+oldSize);
        if (buffer.size() > minV || finish) {
            auto writtenBytes = writer.write(buffer, finish);
            if (writtenBytes > 0) {
                std::copy(buffer.begin()+writtenBytes, buffer.end(), buffer.begin());
                buffer.resize(buffer.size() - writtenBytes);
            }
        }
    }

};

//static_assert(writer_and_dropper_c<buffered_writer<file_writer>>);
}
