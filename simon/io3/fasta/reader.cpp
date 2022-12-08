#include "reader.h"

#include "reader_impl.h"
#include "../file_reader.h"
#include "../mmap_reader.h"
#include "../stream_reader.h"
#include "../zlib_file_reader.h"
#include "../zlib_mmap2_reader.h"
#include "../zlib_ng_file_reader.h"

namespace io3::fasta {
struct reader_pimpl {
    fasta_reader_impl reader;
    reader_pimpl(std::filesystem::path file, bool)
        : reader {[&]() {
            if (file.extension() == ".fa") {
                return fasta_reader_impl{mmap_reader{file.c_str()}};
            } else if (file.extension() == ".gz") {
                return fasta_reader_impl{buffered_reader{zlib_reader{mmap_reader{file.c_str()}}}};
            }
            throw std::runtime_error("unknown file extension");
        }()}
    {}
    reader_pimpl(std::istream& file, bool compressed)
        : reader {[&]() {
            if (!compressed) {
                return fasta_reader_impl{buffered_reader{stream_reader{file}}};
            } else {
                return fasta_reader_impl{buffered_reader{zlib_reader{stream_reader{file}}}};
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

auto begin(reader& _reader) -> reader::iter {
    auto& r= _reader.pimpl->reader;
    return {[&]() { return r.next(); }};
}

}
