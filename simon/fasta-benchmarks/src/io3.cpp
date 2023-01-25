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

template <typename Reader>
void benchmark_io3(Reader&& reader) {
    std::array<int, 5> ctChars{};
    for (auto && [id, seq] : reader) {
        for (auto c : seq | seq_cleanuped_view) {
            assert(c < ctChars.size());
            ctChars[c] += 1;
        }
    }

    size_t a{};
    for (size_t i{0}; i<ctChars.size(); ++i) {
        std::cout << i << ": " << ctChars[i] << "\n";
        a += ctChars[i];
    }
    std::cout << "total: " << a << "\n";
}

template <typename Reader, typename Writer>
void benchmark_io3(Reader&& reader, Writer&& writer) {
    for (auto && record : reader) {
        writer.write(record);
    }
}



void io3_bench(std::string_view _file, std::string_view _method) {
    auto file   = std::string{_file};
    auto method = std::string{_method};
    auto ext = file.substr(file.size() - 3);

    if (method == "io3") {
        benchmark_io3(io3::fasta::reader{{.input = file}});
    } else if (method == "io3_stream" and ext == ".fa") {
        auto ifs = std::ifstream{file.c_str()};
        benchmark_io3(io3::fasta::reader{{.input = ifs}});
    } else if (method == "io3_stream" and ext == ".gz") {
        auto ifs = std::ifstream{file.c_str()};
        benchmark_io3(io3::fasta::reader{{.input = ifs, .compressed = true}});
    } else if (method == "io3_read_write") {
        auto reader = io3::fasta::reader{{.input = file}};
        auto writer = io3::fasta::writer{{.output = file + ".out" + ext}};
        benchmark_io3(reader, writer);
    } else if (method == "io3_read_write_stream" and ext == ".fa") {
        auto ifs = std::ifstream{file.c_str()};
        auto ofs = std::ofstream{file + ".out.fa"};
        auto reader = io3::fasta::reader{{.input = ifs, .compressed = false}};
        auto writer = io3::fasta::writer{{.output = ofs, .compressed = false}};
        benchmark_io3(reader, writer);
    } else if (method == "io3_read_write_stream" and ext == ".gz") {
        auto ifs = std::ifstream{file.c_str()};
        auto ofs = std::ofstream{file + ".out.fa.gz"};
        auto reader = io3::fasta::reader{{.input = ifs, .compressed = true}};
        auto writer = io3::fasta::writer{{.output = ofs, .compressed = true}};
        benchmark_io3(reader, writer);
    } else {
        throw std::runtime_error("invalid method: " + method);
    }
}
