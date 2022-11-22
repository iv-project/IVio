#include "writer.h"

#include "../file_writer.h"

#include <cassert>

namespace io3::fasta {

struct writer_pimpl {
    using Writers = std::variant<file_writer/*,
                                 fasta_reader_impl<buffered_reader<stream_reader>>,
                                 fasta_reader_impl<buffered_reader<zlib_mmap_reader>>,
                                 fasta_reader_impl<buffered_reader<zlib_stream_reader>>*/
                                 >;

    writer_config config;
    Writers writer;
    writer_pimpl(writer_config config)
        : config{config}
        , writer {[&]() -> Writers {
            if (auto ptr = std::get_if<std::filesystem::path>(&config.output)) {
                if (ptr->extension() == ".fa") {
                    return file_writer{ptr->c_str()};
                }/* else if (file.extension() == ".gz") {
                    return fasta_reader_impl{buffered_reader{zlib_mmap_reader{file.c_str()}}};
                }*/
            }
            throw std::runtime_error("unknown output type");
        }()}
    {}
};


writer::writer(writer_config config)
    : pimpl{std::make_unique<writer_pimpl>(config)}
{
    assert(config.length > 0);
}
writer::~writer() = default;


void writer::write(record_view record) {
    assert(pimpl);
    std::visit([&](auto& writer) {
        auto const& config = pimpl->config;
        auto buffer = std::string{};
        buffer.reserve(
            record.id.size() + 2 +                                    // id name + trailing line break
            record.seq.size() + record.seq.size() / config.length + 1 // seq name + breaking every x characters
        );
        buffer = '>';
        buffer += record.id;
        buffer += '\n';
        auto seq = record.seq;
        while (seq.size() > config.length) {
            buffer += seq.substr(0, config.length);
            buffer += '\n';
            seq = seq.substr(config.length);
        }
        if (seq.size() > 0) {
            buffer += seq;
            buffer += '\n';
        }
        writer.write(buffer);
    }, pimpl->writer);
}

}
