#include "Result.h"

#include <ivio/mmap_reader.h>

constexpr static auto ccmap = []() {
    std::array<uint8_t, 256> c;
    c.fill(0xff);
    c['A'] = 0;
    c['C'] = 1;
    c['G'] = 2;
    c['T'] = 3;
    c['N'] = 4;
    c['a'] = 0;
    c['c'] = 1;
    c['g'] = 2;
    c['t'] = 3;
    c['n'] = 4;
    c['\n'] = 0xff;
    c['\r'] = 0xff;
    c[' '] = 0xff;
    c['\t'] = 0xff;

    return c;
}();

auto direct_bench(std::filesystem::path path) -> Result {
    auto reader = ivio::mmap_reader(path.c_str());

    std::array<int, 256> ctChars{};

    reader.dropUntil(reader.readUntil('>', 0)); // drop header

    while (!reader.eof(0)) {
        auto [ptr, size] = reader.read(std::numeric_limits<uint64_t>::max());
        auto buffer = std::string_view{ptr, size};
        auto pos = buffer.find('\n', 0);
        if (pos == std::string_view::npos) break;
        if (++pos >= size) break;
        for(;pos < size and buffer[pos] != '>'; ++pos) {
            assert(pos < size);
            assert(buffer[pos] < 256);
            auto c = ccmap[buffer[pos]];
            ctChars[c] += 1;
        }
        reader.dropUntil(pos);
    }
    Result result;
    for (size_t i : {0, 1, 2, 3, 4}) {
        result.ctChars[i] = ctChars[i];
    }
    return result;
}
