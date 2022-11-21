#include "fasta_reader.h"

#include "fasta_reader_impl.h"
#include "file_reader.h"
#include "mmap_reader.h"
#include "stream_reader.h"
#include "zlib_file_reader.h"
#include "zlib_mmap2_reader.h"
#include "zlib_ng_file_reader.h"

#include <variant>

namespace io3 {
struct fasta_reader_pimpl {
    using Readers = std::variant<fasta_reader_impl<mmap_reader>,
                                 //fasta_reader_impl<stream_reader>,
                                 fasta_reader_impl<buffered_reader<zlib_mmap_reader>>
                                 //fasta_reader_impl<zlib_stream_reader>,
                                 >;
    Readers reader;
    fasta_reader_pimpl(std::filesystem::path file)
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

fasta_reader::fasta_reader(fasta_reader_config config)
    : pimpl{std::make_unique<fasta_reader_pimpl>(config.input)}
{}
fasta_reader::~fasta_reader() = default;

auto begin(fasta_reader& reader) -> fasta_reader::iter {
    return fasta_reader::iter{std::visit([](auto& reader) -> std::function<std::optional<fasta::record_view>()>{
        return [&reader]() { return reader.next(); };
    }, reader.pimpl->reader)};
}

}
