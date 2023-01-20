#include <bio/var_io/reader.hpp>

void bio_bench(std::string_view _file) {

    std::filesystem::path file{_file};

    size_t ct{}, sum{}, l{};

    auto fin  = bio::var_io::reader{file};
    for (auto & r : fin) {
        l += 1;
        ct += r.pos();
//        std::cout << l << " " << ct << "\n";

        for (auto c : r.ref()) {
            sum += c.to_rank();
        }
    }
    std::cout << "total: " << ct << " " << sum << " " << l << "\n";
}
