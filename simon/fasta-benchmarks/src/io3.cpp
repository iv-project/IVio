#include "Result.h"

#include <iostream>
#include <cassert>
#include <fstream>
#include <io3/fasta/reader.h>
#include <io3/fasta/writer.h>
#include <ranges>

constexpr static auto ccmap = []() {
    std::array<uint8_t, 256> c;
    c.fill(0xff);
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

    return c;
}();

inline constexpr auto rank_view = std::views::transform([](char c) {
    auto rank = ccmap[reinterpret_cast<uint8_t&>(c)];
    if (rank == 0xff) {
        throw "invalid character";
    }
    return rank;
});

template <typename Reader>
static auto benchmark_io3(Reader&& reader) -> Result {
    Result result;
    for (auto && [id, seq] : reader) {
        for (auto c : seq | rank_view) {
            assert(c < ctChars.size());
            result.ctChars[c] += 1;
        }
    }
    return result;
}

template <typename Reader, typename Writer>
static void benchmark_io3(Reader&& reader, Writer&& writer) {
    for (auto && record : reader) {
        writer.write(record);
    }
}



auto io3_bench(std::string_view _file, std::string_view _method) -> Result {
    auto file   = std::string{_file};
    auto method = std::string{_method};
    auto ext = file.substr(file.size() - 3);

    if (method == "io3") {
        return benchmark_io3(io3::fasta::reader{{.input = file}});
    } else if (method == "io3_stream" and ext == ".fa") {
        auto ifs = std::ifstream{file.c_str()};
        return benchmark_io3(io3::fasta::reader{{.input = ifs}});
    } else if (method == "io3_stream" and ext == ".gz") {
        auto ifs = std::ifstream{file.c_str()};
        return benchmark_io3(io3::fasta::reader{{.input = ifs, .compressed = true}});
/*    } else if (method == "io3_read_write") {
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
        benchmark_io3(reader, writer);*/
    } else {
        throw std::runtime_error("invalid method: " + method);
    }
}
