#include <ivio/vcf/reader.h>
#include <ivio/vcf/writer.h>

void ivio_bench(std::filesystem::path pathIn, std::filesystem::path pathOut) {
    auto reader = ivio::vcf::reader{{.input = pathIn}};


    auto writer = ivio::vcf::writer{{.output = pathOut,
                                     /*.header = reader.header()*/ }};

    for (auto record_view : reader) {
        writer.write(record_view);
    }
}
