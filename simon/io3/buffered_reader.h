#pragma once

#include "file_reader.h"

namespace io3 {

template <typename Reader, size_t minV = 2<<12>
class buffered_reader : public Reader {
    std::vector<char> buf = []() { auto vec = std::vector<char>{}; vec.reserve(minV); return vec; }();
    int inPos{};

public:
    using Reader::Reader;
    buffered_reader(Reader&& _other)
        : Reader{std::move(_other)}
    {}
    buffered_reader(buffered_reader const&) = delete;
    buffered_reader(buffered_reader&& _other) = default;

private:
    auto readMore() -> bool {
        size_t lastSize = buf.size();
        if (buf.capacity() - buf.size() >= minV) {
            buf.resize(buf.capacity());
        } else {
            buf.resize(buf.capacity()*2);
        }

        auto bytes_read = Reader::read(std::ranges::subrange{buf.data() + lastSize, &*buf.end()});
        buf.resize(lastSize + bytes_read);

        return bytes_read != 0;
    }

public:
    size_t readUntil(char c, size_t lastUsed) {
        while (true) {
            auto ptr = (char const*)memchr(buf.data() + lastUsed + inPos, c, buf.size() - lastUsed - inPos);
            if (ptr != nullptr) {
                return ptr - buf.data() - inPos;
            }
            if (!readMore()) {
                return buf.size() - inPos;
            }
        }
    }

    auto read(size_t ct) -> std::tuple<char const*, size_t> {
        while (buf.size()-inPos < ct) {
            if (!readMore()) break;
        }
        return {buf.data()+inPos, buf.size()-inPos};
    }

    void dropUntil(size_t i) {
        i = i + inPos;
        if (i < minV) {
            inPos = i;
            return;
        }
        std::copy(begin(buf)+i, end(buf), begin(buf));
        buf.resize(buf.size()-i);
        inPos = 0;
    }

    bool eof(size_t i) const {
        return i+inPos == buf.size();
    }

    auto string_view(size_t start, size_t end) -> std::string_view {
        return std::string_view{buf.data()+start+inPos, buf.data()+end+inPos};
    }
};

static_assert(reader_and_dropper_c<buffered_reader<file_reader>>);
}
