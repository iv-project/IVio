#include "Result.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <sys/resource.h>

auto seqan2_bench(std::string_view file) -> Result;
auto seqan3_bench(std::string_view file) -> Result;
auto io2_bench(std::string_view file) -> Result;
auto io2_copy_bench(std::string_view file) -> Result;
auto bio_bench(std::string_view file) -> Result;
auto io3_bench(std::string_view file, std::string_view method) -> Result;
auto io3_mt_bench(std::string_view file, std::string_view method) -> Result;
auto direct_bench(std::filesystem::path path) -> Result;
auto extreme_bench(std::filesystem::path path) -> Result;

void print_results(Result const& result, uint64_t timeInMs) {
    size_t a{};
    for (size_t i{0}; i<result.ctChars.size(); ++i) {
        std::cout << i << ": " << result.ctChars[i] << "\n";
        a += result.ctChars[i];
    }
    std::cout << "total: " << a << "\n";
    std::cout << a / 1000 / timeInMs << " MB/s\n";

    rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    std::cout << "Memory:" << usage.ru_maxrss/1024 << " MB\n";
}

int main(int argc, char** argv) {
    try {
        if (argc != 3) return 0;
        auto method = std::string_view{argv[1]};
        auto file   = std::string_view{argv[2]};
        auto start  = std::chrono::high_resolution_clock::now();

        auto r = [&]() {
            if (method == "seqan2")           return seqan2_bench(file);
            if (method == "seqan3")           return seqan3_bench(file);
            if (method == "io2")              return io2_bench(file);
            if (method == "io2-copy")         return io2_copy_bench(file);
            if (method == "bio")              return bio_bench(file);
            if (method.starts_with("io3_mt")) return io3_mt_bench(file, method);
            if (method.starts_with("io3"))    return io3_bench(file, method);
            if (method == "direct")           return direct_bench(file);
            if (method == "extreme")          return extreme_bench(file);
            throw std::runtime_error("unknown method: " + std::string{method});
        }();
        auto end  = std::chrono::high_resolution_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        auto groundTruth = io3_bench(file, "io3");
        print_results(method, groundTruth, r, diff);

    } catch (std::exception const& e) {
        std::cout << "exception(e): " << e.what() << "\n";
    }
}
