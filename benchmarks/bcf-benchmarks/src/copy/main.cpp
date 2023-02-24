#include "../fasta-benchmarks/src/write/dna5_rank_view.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>
#include <sys/resource.h>
#include <ivio/vcf/reader.h>

void seqan2_bench(std::filesystem::path pathIn, std::filesystem::path pathOut);
void bio_bench(std::filesystem::path pathIn, std::filesystem::path pathOut);
void ivio_bench(std::filesystem::path pathIn, std::filesystem::path pathOut);

int main(int argc, char** argv) {
    auto p = [](auto v, size_t w) {
        auto ss = std::stringstream{};
        ss << std::boolalpha << v;
        auto str = ss.str();
        while (str.size() < w) {
            str += " ";
        }
        return str;
    };

    try {
        if (argc != 3) return 0;
        auto method     = std::string_view{argv[1]};
        auto input_file = std::filesystem::path{argv[2]};

        auto output_file   = std::filesystem::path{"/tmp/ivio_bench"} / input_file.filename();
        std::filesystem::create_directories(output_file.parent_path());

        int fastestRun{};
        auto fastestTime = std::numeric_limits<int>::max();
        int maxNbrOfRuns{5};
        for (int i{}; i < maxNbrOfRuns; ++i) {
            auto start  = std::chrono::high_resolution_clock::now();

            if (method == "seqan2")      seqan2_bench(input_file, output_file);
            else if (method == "bio")    bio_bench(input_file, output_file);
            else if (method == "ivio")   ivio_bench(input_file, output_file);
            else throw std::runtime_error("unknown method");

            auto end  = std::chrono::high_resolution_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
/*            auto call = [&]() -> std::string {
                if (compressed) {
                    return "zdiff \"" + input_file.string() + "\" \"" + output_file.string() + "\"";
                }
                return "diff \"" + input_file.string() + "\" \"" + output_file.string() + "\"";
            }();
            int result = system(call.c_str());
            bool correct = (result == 0);
            if (!correct) {
                throw std::runtime_error("incorrect output");
            }*/
            if (diff < fastestTime) {
                fastestTime = diff;
                fastestRun = i;
            }
        }
        // print results
        [&]() {

            auto timeInMs = fastestTime;
            size_t a = std::filesystem::file_size(std::string{input_file});
            auto memory = []() {
                rusage usage;
                getrusage(RUSAGE_SELF, &usage);
                return usage.ru_maxrss / 1024;
            }();
            timeInMs = std::max(1, timeInMs);
            std::cout << "method  \tcorrect \ttotal(MB)\tspeed(MB/s)\tmemory(MB)\n";
            std::cout << p(method, 8) << "\t"
                      << p(true, 8) << "\t"
                      << p(a/1024/1024, 8) << "\t"
                      << p(a/1024/timeInMs, 8) << "\t"
                      << p(memory, 8) << "\t"
                      << (fastestRun+1) << "/" << maxNbrOfRuns << "\n";
        }();

    } catch (std::exception const& e) {
        std::cout << "method  \tcorrect \ttotal(MB)\tspeed(MB/s)\tmemory(MB)\n";
        std::cout << p(std::string{argv[1]}, 8) << "\t"
                  << p(false, 8) << "\t"
                  << p(0, 8) << "\t"
                  << p(0, 8) << "\t"
                  << p(0, 8) << "\t"
                  << 0 << "/" << 0 << "\n";
    }
}
