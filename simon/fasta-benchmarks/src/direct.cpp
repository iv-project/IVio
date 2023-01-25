#include <array>
#include <io3/mmap_reader.h>
#include <iostream>

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

void direct_bench(std::filesystem::path path) {
    auto reader = io3::mmap_reader(path.c_str());

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

    size_t a{};
    for (size_t i{0}; i<5; ++i) {
        std::cout << i << ": " << ctChars[i] << "\n";
        a += ctChars[i];
    }
    std::cout << "total: " << a << "\n";
}

