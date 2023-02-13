#include "Result.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <io3/buffered_reader.h>
#include <io3/stream_reader.h>

auto extreme_bench(std::ifstream& stream, bool compressed) -> Result {
    if (compressed) return {};

    auto reader = io3::buffered_reader{io3::stream_reader{stream}};
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
    result.ctChars[2] = ctChars['G'] + ctChars['g'];
    result.ctChars[3] = ctChars['T'] + ctChars['t'];
    result.ctChars[4] = ctChars['N'] + ctChars['n'];
    return result;
}
