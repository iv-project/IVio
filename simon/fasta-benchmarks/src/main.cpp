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
#include <filesystem>


void seqan2_bench(std::string_view file);
void seqan3_bench(std::string_view file);
void io2_bench(std::string_view file);
void io2_copy_bench(std::string_view file);
void bio_bench(std::string_view file);
void io3_bench(std::string_view file, std::string_view method);
void direct_bench(std::filesystem::path path);

int main(int argc, char** argv) {
    try {
        if (argc != 3) return 0;
        auto method = std::string_view{argv[1]};
        auto file   = std::string_view{argv[2]};

        if (method == "seqan2") {
            seqan2_bench(file);
        } else if (method == "seqan3") {
            seqan3_bench(file);
        } else if (method == "io2") {
            io2_bench(file);
        } else if (method == "io2-copy") {
            io2_copy_bench(file);
        } else if (method == "bio") {
            bio_bench(file);
        } else if (method.starts_with("io3")) {
            io3_bench(file, method);
        } else if (method == "direct") {
            direct_bench(file);
        } else {
            throw std::runtime_error("unknown method: " + std::string{method});
        }
    } catch (std::exception const& e) {
        std::cout << "exception(e): " << e.what() << "\n";
    }
}
