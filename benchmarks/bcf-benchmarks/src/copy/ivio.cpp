#include <ivio/bcf/reader.h>
#include <ivio/bcf/writer.h>

void ivio_bench(std::filesystem::path pathIn, std::filesystem::path pathOut) {
    auto reader = ivio::bcf::reader{{.input = pathIn}};
    auto writer = ivio::bcf::writer{{.output = pathOut,
                                     .header = reader.header() }};

    for (auto record_view : reader) {
        writer.write(record_view);
    }
}
