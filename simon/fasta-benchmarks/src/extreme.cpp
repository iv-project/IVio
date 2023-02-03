#include "Result.h"

#include <io3/mmap_reader.h>

constexpr static auto ccmap = []() {
    std::array<uint8_t, 256> c;
    c.fill(3);
    c['A'] = 0;
    c['C'] = 1;
    c['G'] = 2;
    c['N'] = 3;
    c['T'] = 4;
    c['a'] = 0;
    c['c'] = 1;
    c['g'] = 2;
    c['n'] = 3;
    c['t'] = 4;
    c['\n'] = 0xff;
    c['\r'] = 0xff;
    c[' '] = 0xff;
    c['\t'] = 0xff;

    return c;
}();

auto extreme_bench(std::filesystem::path path) -> Result {
    auto reader = io3::mmap_reader(path.c_str());

    std::array<int, 256> ctChars{};

    auto [ptr, size] = reader.read(std::numeric_limits<uint64_t>::max());
    auto buffer = std::string_view{ptr, size};

    size_t pos = buffer.find('>');
    if (pos != std::string_view::npos) {
        while (pos < size) {
            pos = buffer.find('\n', pos);
            if (pos == std::string_view::npos) break;
            if (++pos >= size) break;
            for(;pos < size and buffer[pos] != '>'; ++pos) {
                assert(pos < size);
                assert(buffer[pos] < 256);
                ctChars[buffer[pos]] += 1;
            }
        }
    }



    Result result;
    result.ctChars[0] = ctChars['A'] + ctChars['a'];
    result.ctChars[1] = ctChars['C'] + ctChars['c'];
    result.ctChars[2] = ctChars['G'] + ctChars['G'];
    result.ctChars[3] = ctChars['N'] + ctChars['n'];
    result.ctChars[4] = ctChars['T'] + ctChars['t'];
    return result;
}
