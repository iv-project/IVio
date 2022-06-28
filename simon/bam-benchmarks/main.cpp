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
#include <ranges>

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

void seqan2_bench(std::string const& file);
void seqan3_bench(std::string const& file);

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
    }


/*    if (method == "view" and ext == ".gz") {
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
    } else if (method == "best") {
        benchmark(fasta_reader_best{file});
    } else*/ {
        std::cout << "unknown\n";
    }
//    } catch(char const* what) {
//        std::cout << "exception(c): " << what << "\n";
//    } catch(std::string const& what) {
//        std::cout << "exception(s): " << what << "\n";
//    }
}
