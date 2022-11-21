#include "fasta_reader.h"

#include "fasta_reader_impl.h"
#include "file_reader.h"
#include "mmap_reader.h"
#include "stream_reader.h"
#include "zlib_file_reader.h"
#include "zlib_mmap2_reader.h"
#include "zlib_ng_file_reader.h"

#include <variant>

namespace io3::fasta {
struct reader_pimpl {
    using Readers = std::variant<fasta_reader_impl<mmap_reader>,
                                 //fasta_reader_impl<stream_reader>,
                                 fasta_reader_impl<buffered_reader<zlib_mmap_reader>>
                                 //fasta_reader_impl<zlib_stream_reader>,
                                 >;
    Readers reader;
    reader_pimpl(std::filesystem::path file)
        : reader {[&]() -> Readers {
            if (file.extension() == ".fa") {
                return fasta_reader_impl{mmap_reader{file.c_str()}};
            } else if (file.extension() == ".gz") {
                return fasta_reader_impl{buffered_reader{zlib_mmap_reader{file.c_str()}}};
            }
            throw std::runtime_error("unknown file extension");
        }()}
    {}
};

reader::reader(reader_config config)
    : pimpl{std::make_unique<reader_pimpl>(config.input)}
{}
reader::~reader() = default;

auto begin(reader& _reader) -> reader::iter {
    return reader::iter{std::visit([](auto& _reader) -> std::function<std::optional<fasta::record_view>()>{
        return [&_reader]() { return _reader.next(); };
    }, _reader.pimpl->reader)};
}

}
