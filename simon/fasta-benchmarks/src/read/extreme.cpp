#include "Result.h"

#include <io3/mmap_reader.h>

auto extreme_bench(std::filesystem::path path) -> Result {
    auto reader = io3::mmap_reader(path);

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
