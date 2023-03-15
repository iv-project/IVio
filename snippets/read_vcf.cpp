#include <ivio/vcf/reader.h>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }

    auto file = std::filesystem::path{argv[1]};
    for (auto view : ivio::vcf::reader{{file}}) {
        std::cout << view.ref << "\n";
    }
}
