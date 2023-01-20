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


void seqan2_bench(std::string_view file);
void bio_bench(std::string_view file);
void io3_bench(std::string_view method, std::string_view file);

int main(int argc, char** argv) {
    try {
        if (argc != 3) return 0;
        auto method = std::string_view{argv[1]};
        auto file   = std::string_view{argv[2]};

        if (method == "seqan2") {
            seqan2_bench(file);
            return 0;
        } else if (method == "bio") {
            bio_bench(file);
            return 0;
        } else if (method.starts_with("io3")) {
            io3_bench(method, file);
        }
    } catch(char const* what) {
        std::cout << "exception(c): " << what << "\n";
    } catch(std::string const& what) {
        std::cout << "exception(s): " << what << "\n";
    }
}
