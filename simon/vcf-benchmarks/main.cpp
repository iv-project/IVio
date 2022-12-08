#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <optional>
#include <ranges>
#include <unistd.h>
#include <variant>
#include <vector>

#include "io3/io3.h"

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


inline constexpr auto seq_cleanuped_view = std::views::transform([](char c) {
            return ccmap[reinterpret_cast<uint8_t&>(c)];
        }) | std::views::filter([](char c) {
            if (c == (char)0xff) {
                throw "invalid variable";
            }
            return true;
        });


template <typename Reader>
void benchmark_io3(Reader&& reader) {
    size_t ct{}, sum{}, l{};
    for (auto && view : reader) {
        l += 1;
        ct += view.pos;
//        std::cout << l << " " << ct << " " << view.pos << "\n";
        for (auto c : view.ref | seq_cleanuped_view) {
            sum += c;
        }
  //      if (l > 800) exit(0);
    }
    std::cout << "total: " << ct << " " << sum << " " << l << "\n";
}

template <typename Reader, typename Writer>
void benchmark_io3(Reader& reader, Writer& writer) {
    for (auto const& [key, value] : reader.header) {
        writer.writeHeader(key, value);
    }
    for (auto gt : reader.genotypes) {
        writer.addGenotype(gt);
    }
    for (auto && view : reader) {
        writer.write(view);
    }
}

template <typename Reader, typename Writer>
void benchmark_io3_bcf(Reader& reader, Writer& writer) {
    writer.writeHeader(reader.headerBuffer);
    for (size_t i{0}; i < reader.contigMap.size(); ++i) {
        writer.contigMap.try_emplace(std::string{reader.contigMap[i]}, i);
    }
    for (size_t i{0}; i < reader.filterMap.size(); ++i) {
        writer.filterMap.try_emplace(std::string{reader.filterMap[i]}, i);
    }

    for (auto && view : reader) {
        writer.write(view);
    }
}



void seqan2_bench(std::string const& file);
void bio_bench(std::string const& file);

int main(int argc, char** argv) {
    try {
    if (argc != 3) return 0;
    std::string method = argv[1];
    std::string file = argv[2];

    auto ext = file.substr(file.size() - 4);

    if (method == "seqan2") {
        seqan2_bench(file);
        return 0;
    } else if (method == "bio") {
        bio_bench(file);
        return 0;
    }


    if (method == "io3_file" and ext == ".vcf") {
        benchmark_io3(io3::vcf::reader{io3::file_reader(file.c_str())});
    } else if (method == "io3_mmap" and ext == ".vcf") {
        benchmark_io3(io3::vcf::reader{io3::mmap_reader(file.c_str())});
    } else if (method == "io3_stream" and ext == ".vcf") {
        auto ifs = std::ifstream{file.c_str()};
        benchmark_io3(io3::vcf::reader{io3::stream_reader(ifs)});
    } else if (method == "io3_copy" and ext == ".vcf") {
        auto reader = io3::vcf::reader{io3::file_reader(file.c_str())};
        auto writer = io3::vcf::writer{{.output = file + ".out.vcf"}};
        benchmark_io3(reader, writer);
    } else if (method == "io3_file" and ext == ".bcf") {
         benchmark_io3(io3::bcf::reader{io3::bgzf_file_reader{file.c_str()}});
    } else if (method == "io3_mmap" and ext == ".bcf") {
         benchmark_io3(io3::bcf::reader{io3::bgzf_mmap_reader{file.c_str()}});
    } else if (method == "io3_stream" and ext == ".bcf") {
        auto ifs = std::ifstream{file.c_str()};
        benchmark_io3(io3::bcf::reader{io3::bgzf_stream_reader(ifs)});
    } else if (method == "io3_copy" and ext == ".bcf") {
        auto reader = io3::bcf::reader{io3::bgzf_file_reader{io3::file_reader(file.c_str())}};
        auto writer = io3::bcf::writer{{.output = file + ".out.bcf"}};
        benchmark_io3_bcf(reader, writer);
    } else {
        std::cout << "unknown\n";
    }
    } catch(char const* what) {
        std::cout << "exception(c): " << what << "\n";
    } catch(std::string const& what) {
        std::cout << "exception(s): " << what << "\n";
    }
}
