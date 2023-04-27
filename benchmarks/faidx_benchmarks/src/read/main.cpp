// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file.
// -----------------------------------------------------------------------------------------------------
#include "Result.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <sys/resource.h>

auto ivio_bench(std::filesystem::path file) -> Result;

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
        auto method = std::string_view{argv[1]};
        auto file   = std::filesystem::path{argv[2]};

        Result bestResult;
        int fastestRun{};
        auto fastestTime = std::numeric_limits<int>::max();
        int maxNbrOfRuns{5};
        for (int i{}; i < maxNbrOfRuns; ++i) {
            auto start  = std::chrono::high_resolution_clock::now();

            auto r = [&]() {
                if (method == "ivio")             return ivio_bench(file);
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
        auto groundTruth = ivio_bench(file);
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
            std::cout << "method  \tcorrect \ttotal(MB)\tspeed(MB/s)\tmemory(MB)\n";
            std::cout << p(method, 8) << "\t"
                      << p(correct, 8) << "\t"
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
