#include "Result.h"
#include "../fasta-benchmarks/src/read/dna5_rank_view.h"

#include <io3/vcf/reader.h>

auto io3_bench(std::string_view _file) -> Result {
    auto file = std::string{_file};

    Result result;
    for (auto && view : io3::vcf::reader{{file}}) {
        result.l += 1;
        result.ct += view.pos;
        for (auto c : view.ref | dna5_rank_view) {
            result.ctChars[c] += 1;
        }
        result.bytes += view.ref.size();
    }
    return result;
}

/*constexpr static std::array<char, 256> ccmap = []() {
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
        });*/

/*
template <typename Reader>
auto benchmark_io3(Reader&& reader) -> Result {
    Result result;
    for (auto && view : reader) {
        result.l += 1;
        result.ct += view.pos;
        for (auto c : view.ref | seq_cleanuped_view) {
            result.sum += c;
        }
    }
    return result;
}*/

/*template <typename Reader, typename Writer>
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
}*/


/*void io3_bench(std::string_view _file) {
    auto file = std::string{_file};

    Result result;
    for (auto && view : io3::vcf::reader{{file}}) {
        result.l += 1;
        result.ct += view.pos;
        for (auto c : view.ref | seq_cleanuped_view) {
            result.sum += c;
        }
    }
    return result;


    if (ext == ".vcf") {
        benchmark_io3(io3::vcf::reader{{file}});
    } else if (method == "io3_copy" and ext == ".vcf") {
        auto reader = io3::vcf::reader{{file}};
        auto writer = io3::vcf::writer{{.output = file + ".out.vcf"}};
        benchmark_io3(reader, writer);
    } else if (method == "io3_file" and ext == ".bcf") {
        benchmark_io3(io3::bcf::reader{io3::buffered_reader<1<<16>{io3::bgzf_reader{io3::file_reader{file.c_str()}}}});
    } else if (method == "io3_mmap" and ext == ".bcf") {
        benchmark_io3(io3::bcf::reader{io3::buffered_reader<1<<16>{io3::bgzf_reader{io3::mmap_reader{file.c_str()}}}});
    } else if (method == "io3_stream" and ext == ".bcf") {
        auto ifs = std::ifstream{file.c_str()};
        benchmark_io3(io3::bcf::reader{io3::buffered_reader<1<<16>{io3::bgzf_reader(io3::stream_reader{ifs})}});
    } else if (method == "io3_copy" and ext == ".bcf") {
        auto reader = io3::bcf::reader{io3::buffered_reader<1<<16>{io3::bgzf_reader{io3::file_reader(file.c_str())}}};
        auto writer = io3::bcf::writer{{.output = file + ".out.bcf"}};
        benchmark_io3_bcf(reader, writer);
    } else {
        std::cout << "unknown\n";
    }
}*/
