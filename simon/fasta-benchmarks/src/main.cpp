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

template <typename... Ts>
void noOpt(Ts&&...) {
    asm("");
}

template <typename Reader>
void benchmark(Reader&& reader) {
    std::array<int, 5> ctChars{};
    for (auto && [id, seq] : reader) {
        for (auto c : seq) {
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


void benchmarkDirect(std::filesystem::path path) {
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


void seqan2_bench(std::string const& file);
void seqan3_bench(std::string const& file);
void io2_bench(std::string const& file);
void io2_copy_bench(std::string const& file);
void bio_bench(std::string const& file);

int main(int argc, char** argv) {
//    try {
    if (argc != 3) return 0;
    std::string method = argv[1];
    std::string file = argv[2];

    auto ext = file.substr(file.size() - 3);

    if (method == "seqan2") {
        seqan2_bench(file);
        return 0;
    } else if (method == "seqan3") {
        seqan3_bench(file);
        return 0;
    } else if (method == "io2") {
        io2_bench(file);
        return 0;
    } else if (method == "io2-copy") {
        io2_copy_bench(file);
        return 0;
    } else if (method == "bio") {
        bio_bench(file);
        return 0;
    }


    if (method == "direct" and ext == ".fa") {
        benchmarkDirect(file);
    } else if (method == "io3") {
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

    } else if (method == "view" and ext == ".gz") {
        benchmark(fasta_reader_view{zlib_reader(file.c_str())});
    } else if (method == "cont" and ext == ".gz") {
        benchmark(fasta_reader_contigous{zlib_reader(file.c_str())});
    } else if (method == "view" and ext == ".fa") {
        benchmark(fasta_reader_view{file_reader(file.c_str())});
    } else if (method == "cont" and ext == ".fa") {
        benchmark(fasta_reader_contigous{file_reader(file.c_str())});
    } else if (method == "mmap_view" and ext == ".fa") {
        benchmark(fasta_reader_mmap{mmap_file_reader(file.c_str())});
    } else if (method == "mmap_view" and ext == ".gz") {
        benchmark(fasta_reader_view{zlib_mmap_reader(file.c_str())});
    } else if (method == "mmap_cont" and ext == ".gz") {
        benchmark(fasta_reader_contigous{zlib_mmap_reader(file.c_str())});
    } else if (method == "mmap_view2" and ext == ".gz") {
        benchmark(fasta_reader_mmap2{zlib_mmap_reader(file.c_str())});
    } else {
        std::cout << "unknown\n";
    }
//    } catch(char const* what) {
//        std::cout << "exception(c): " << what << "\n";
//    } catch(std::string const& what) {
//        std::cout << "exception(s): " << what << "\n";
//    }
}
