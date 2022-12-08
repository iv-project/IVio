#include "reader.h"

#include "../buffered_reader.h"
#include "../file_reader.h"
#include "../mmap_reader.h"
#include "../stream_reader.h"
#include "../zlib_file_reader.h"
#include "../zlib_mmap2_reader.h"
#include "../zlib_ng_file_reader.h"

namespace io3::fasta {
struct reader_pimpl {
    VarBufferedReader reader;
    size_t lastUsed{};
    std::string s;

    reader_pimpl(std::filesystem::path file, bool)
        : reader {[&]() -> VarBufferedReader {
            if (file.extension() == ".fa") {
                return mmap_reader{file.c_str()};
            } else if (file.extension() == ".gz") {
                return zlib_reader{mmap_reader{file.c_str()}};
            }
            throw std::runtime_error("unknown file extension");
        }()}
    {}
    reader_pimpl(std::istream& file, bool compressed)
        : reader {[&]() -> VarBufferedReader {
            if (!compressed) {
                return stream_reader{file};
            } else {
                return zlib_reader{stream_reader{file}};
            }
        }()}
    {}

};

reader::reader(reader_config config)
    : pimpl{std::visit([&](auto& p) {
        return std::make_unique<reader_pimpl>(p, config.compressed);
    }, config.input)}
{}
reader::~reader() = default;

auto reader::next() -> std::optional<record_view> {
    auto& reader   = pimpl->reader;
    auto& lastUsed = pimpl->lastUsed;
    auto& s        = pimpl->s;

    auto startId = reader.readUntil('>', lastUsed);
    if (reader.eof(startId)) return std::nullopt;
    reader.dropUntil(startId+1);

    auto endId = reader.readUntil('\n', 0);
    if (reader.eof(endId)) return std::nullopt;

    auto startSeq = endId+1;

    // convert into dense string representation
    s.clear();
    {
        auto s2 = startSeq;
        do {
            auto s1 = s2;
            s2 = reader.readUntil('\n', s1);
            s += reader.string_view(s1, s2);
            s2 += 1;
        } while (!reader.eof(s2) and reader.string_view(s2, s2+1)[0] != '>');
        lastUsed = s2;
    }


    return record_view {
        .id  = reader.string_view(0,        endId),
        .seq = s,
    };
}

auto begin(reader& _reader) -> reader::iter {
    return {_reader};
}

}
