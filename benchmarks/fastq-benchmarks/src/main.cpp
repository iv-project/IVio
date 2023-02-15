#include "Result.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <sys/resource.h>

auto seqan2_bench(std::string_view file) -> Result;
auto seqan3_bench(std::string_view file) -> Result;
auto bio_bench(std::string_view file) -> Result;
auto io3_bench(std::string_view file) -> Result;



int main(int argc, char** argv) {
    try {
        if (argc != 3) return 0;
        auto method = std::string_view{argv[1]};
        auto file   = std::string_view{argv[2]};

        Result bestResult;
        int fastestRun{};
        auto fastestTime = std::numeric_limits<int>::max();
        int maxNbrOfRuns{5};
        try {
            for (int i{}; i < maxNbrOfRuns; ++i) {
                auto start  = std::chrono::high_resolution_clock::now();

                auto r = [&]() {
                    if (method == "seqan2")   return seqan2_bench(file);
                    if (method == "seqan3")   return seqan3_bench(file);
                    if (method == "bio")      return bio_bench(file);
                    if (method == "io3")      return io3_bench(file);
                    throw std::runtime_error("unknown method: " + std::string{method});
                }();
                auto end  = std::chrono::high_resolution_clock::now();
                auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                if (diff < fastestTime) {
                    bestResult = r;
                    fastestTime = diff;
                    fastestRun = i;
                }
            }
        } catch(...){
            bestResult = Result{}; // reset results, will cause this to be incorrect
        }
        auto groundTruth = io3_bench(file);
        // print results
        [&]() {
            auto const& result = bestResult;
            auto timeInMs = fastestTime;
            bool correct{true};
            size_t a{};
            for (size_t i{0}; i<result.ctChars.size(); ++i) {
                if (groundTruth.ctChars[i] != result.ctChars[i]) {
                    correct = false;
                }
                a += result.ctChars[i];
            }
            auto memory = []() {
                rusage usage;
                getrusage(RUSAGE_SELF, &usage);
                return usage.ru_maxrss / 1024;
            }();
            auto p = [](auto v, size_t w) {
                auto ss = std::stringstream{};
                ss << std::boolalpha << v;
                auto str = ss.str();
                while (str.size() < w) {
                    str += " ";
                }
                return str;
            };
            std::cout << "method  \tcorrect \ttotal(MB)\tspeed(MB/s)\tmemory(MB)\n";
            std::cout << p(method, 8) << "\t"
                      << p(correct, 8) << "\t"
                      << p(a/1024/1024, 8) << "\t"
                      << p(a/1024/timeInMs, 8) << "\t"
                      << p(memory, 8) << "\t"
                      << (fastestRun+1) << "/" << maxNbrOfRuns << "\n";
        }();
    } catch (std::exception const& e) {
        std::cout << "exception(e): " << e.what() << "\n";
    } catch (char const* str) {
        std::cout << "exception(c): " << str << "\n";
    }
}
