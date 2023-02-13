#include "Result.h"
#include "dna5_rank_view.h"

#include <cassert>
#include <fstream>
#include <io3/fasta/reader_mt.h>
#include <io3/fasta/writer.h>

template <typename Reader>
static auto benchmark_io3(Reader&& reader) -> Result {
    Result result;
    for (auto && [id, seq] : reader) {
        for (auto c : seq | dna5_rank_view) {
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

auto io3_mt_bench(std::string_view _file, std::string_view _method) -> Result {
    auto file   = std::string{_file};
    auto method = std::string{_method};
    auto ext = file.substr(file.size() - 3);

    if (method == "io3_mt") {
        return benchmark_io3(io3::fasta::reader_mt{{.input = file}});
    } else if (method == "io3_mt_stream" and ext == ".fa") {
        auto ifs = std::ifstream{file.c_str()};
        return benchmark_io3(io3::fasta::reader_mt{{.input = ifs}});
    } else if (method == "io3_mt_stream" and ext == ".gz") {
        auto ifs = std::ifstream{file.c_str()};
        return benchmark_io3(io3::fasta::reader_mt{{.input = ifs, .compressed = true}});
/*    } else if (method == "io3_mt_read_write") {
        auto reader = io3::fasta::reader_mt{{.input = file}};
        auto writer = io3::fasta::writer{{.output = file + ".out" + ext}};
        benchmark_io3(reader, writer);
    } else if (method == "io3_mt_read_write_stream" and ext == ".fa") {
        auto ifs = std::ifstream{file.c_str()};
        auto ofs = std::ofstream{file + ".out.fa"};
        auto reader = io3::fasta::reader_mt{{.input = ifs, .compressed = false}};
        auto writer = io3::fasta::writer{{.output = ofs, .compressed = false}};
        benchmark_io3(reader, writer);
    } else if (method == "io3_mt_read_write_stream" and ext == ".gz") {
        auto ifs = std::ifstream{file.c_str()};
        auto ofs = std::ofstream{file + ".out.fa.gz"};
        auto reader = io3::fasta::reader_mt{{.input = ifs, .compressed = true}};
        auto writer = io3::fasta::writer{{.output = ofs, .compressed = true}};
        benchmark_io3(reader, writer);*/
    } else {
        throw std::runtime_error("invalid method: " + method);
    }
}
