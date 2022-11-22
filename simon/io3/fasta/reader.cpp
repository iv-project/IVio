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
    using Readers = std::variant<fasta_reader_impl<mmap_reader>,
                                 fasta_reader_impl<buffered_reader<stream_reader>>,
                                 fasta_reader_impl<buffered_reader<zlib_mmap_reader>>,
                                 fasta_reader_impl<buffered_reader<zlib_stream_reader>>
                                 >;
    Readers reader;
    reader_pimpl(std::filesystem::path file, bool)
        : reader {[&]() -> Readers {
            if (file.extension() == ".fa") {
                return fasta_reader_impl{mmap_reader{file.c_str()}};
            } else if (file.extension() == ".gz") {
                return fasta_reader_impl{buffered_reader{zlib_mmap_reader{file.c_str()}}};
            }
            throw std::runtime_error("unknown file extension");
        }()}
    {}
    reader_pimpl(std::istream& file, bool compressed)
        : reader {[&]() -> Readers {
            if (!compressed) {
                return fasta_reader_impl{buffered_reader{stream_reader{file}}};
            } else {
                return fasta_reader_impl{buffered_reader{zlib_stream_reader{file}}};
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
    return reader::iter{std::visit([](auto& _reader) -> std::function<std::optional<fasta::record_view>()>{
        return [&_reader]() { return _reader.next(); };
    }, _reader.pimpl->reader)};
}

}
