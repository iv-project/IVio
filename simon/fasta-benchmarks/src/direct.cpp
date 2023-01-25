#include <cassert>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <array>
#include <algorithm>
#include <optional>
#include <vector>
#include <limits>
#include <numeric>
#include <variant>
#include <iostream>
#include <fstream>

#include "file_reader.h"

#include "mmap_file_reader.h"
#include "zlib_reader.h"
#include "zlib_mmap_reader.h"

#include "io3/fasta/reader.h"
#include "io3/fasta/writer.h"

constexpr static std::array<char, 256> ccmap = []() {
    std::array<char, 256> c;
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
    c['\n'] = (char)0xff;
    c['\r'] = (char)0xff;
    c[' '] = (char)0xff;
    c['\t'] = (char)0xff;

    return c;
}();

#include "fasta_reader_view.h"
#include "fasta_reader_contigous.h"
#include "fasta_reader_mmap.h"
#include "fasta_reader_mmap2.h"
//#include "fasta_reader_best.h"

#include <ranges>
#include <iostream>



void direct_bench(std::filesystem::path path) {
    auto reader = mmap_file_reader(path.c_str());


    auto iter = begin(reader);
    auto end_ = end(reader);

    auto readUntil = [&](char c) {
        auto ptr = (char const*)memchr(iter, c, end_-iter);
        if (ptr == nullptr) {
            return end_;
        }
        return ptr;
    };

    std::array<int, 256> ctChars{};

    iter = readUntil('>');
    while (iter != end_) {
        auto startSeq = readUntil('\n');
        if (startSeq == end_) break;
        if (startSeq+1  == end_) break;
        for (iter = startSeq+1; iter < end_ and *iter != '>'; ++iter) {
            auto c = ccmap[*iter];
            ctChars[c] += 1;
        }
        if (iter - begin(reader) >= 1'024ul * 1'024ul) {
            reader.doneUntil(iter);
        }
    }

    size_t a{};
    for (size_t i{0}; i<5; ++i) {
        std::cout << i << ": " << ctChars[i] << "\n";
        a += ctChars[i];
    }
    std::cout << "total: " << a << "\n";
}

