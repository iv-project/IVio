#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>
#include <sys/resource.h>
#include <io3/fasta/reader.h>
#include "../read/dna5_rank_view.h"

void seqan2_bench(std::filesystem::path file, std::vector<std::tuple<std::string, std::vector<uint8_t>>> const& data);
void seqan3_bench(std::filesystem::path file, std::vector<std::tuple<std::string, std::vector<uint8_t>>> const& data);
void io3_bench(std::filesystem::path file, std::vector<std::tuple<std::string, std::vector<uint8_t>>> const& data);



using Data = std::vector<std::tuple<std::string, std::vector<uint8_t>>>;
static auto loadData(std::filesystem::path const& input_file, bool swap) -> Data {
    auto buffer = Data{};
    for (auto record : io3::fasta::reader{{input_file}}) {
        auto seq_view = record.seq | dna5_rank_view;
        auto seq      = std::vector<uint8_t>{seq_view.begin(), seq_view.end()};
        buffer.emplace_back(std::string{record.id}, seq);
        if (swap) {
            for (auto& c : std::get<1>(buffer.back())) {
                if (c == 3) c = 4;
                else if (c == 4) c = 3;
            }
        }
    }
    return buffer;
}

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

        bool compressed = (input_file.extension() == ".gz");

        auto swap = [&]() {
            if (method == "seqan3") return true;
            return false;
        }();
        auto data = loadData(input_file, swap);

        auto file   = std::filesystem::path{"/tmp/io3_bench"} / input_file.filename();
        std::filesystem::create_directories(file.parent_path());

        int fastestRun{};
        auto fastestTime = std::numeric_limits<int>::max();
        int maxNbrOfRuns{5};
        for (int i{}; i < maxNbrOfRuns; ++i) {
            auto start  = std::chrono::high_resolution_clock::now();

            if (method == "seqan2") seqan2_bench(file, data);
            else if (method == "seqan3") seqan3_bench(file, data);
            else if (method == "io3") io3_bench(file, data);
            else throw std::runtime_error("unknown method");

            auto end  = std::chrono::high_resolution_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            auto call = [&]() -> std::string {
                if (compressed) {
                    return "zdiff \"" + input_file.string() + "\" \"" + file.string() + "\"";
                }
                return "diff \"" + input_file.string() + "\" \"" + file.string() + "\"";
            }();
            int result = system(call.c_str());
            bool correct = (result == 0);
            if (!correct) {
                throw std::runtime_error("incorrect output");
            }
            if (diff < fastestTime and correct) {
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
