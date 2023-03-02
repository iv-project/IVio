#include <ivio/bcf/reader.h>
#include <ivio/bcf/writer.h>

void ivio_bench(std::filesystem::path pathIn, std::filesystem::path pathOut, size_t threadNbr) {
    auto reader = ivio::bcf::reader{{.input     = pathIn,
                                     .threadNbr = threadNbr}};
    auto writer = ivio::bcf::writer{{.output = pathOut,
                                     .header = reader.header() }};

    for (auto record_view : reader) {
        writer.write(record_view);
    }
}
