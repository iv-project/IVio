#include <bio/io/var/reader.hpp>
#include <bio/io/var/writer.hpp>

void bio_bench(std::filesystem::path pathIn, std::filesystem::path pathOut) {
    auto fin  = bio::io::var::reader{pathIn};
    auto fout = bio::io::var::writer{pathOut};

    for (auto & rec : fin) {
        fout.push_back(rec);
    }
}
