#include "writer.h"

#include "../buffered_writer.h"
#include "../file_writer.h"
#include "../stream_writer.h"
#include "../zlib_file_writer.h"

#include <cassert>

namespace io3::fasta {

struct writer_pimpl {
    using Writers = std::variant<file_writer,
                                 buffered_writer<zlib_file_writer>,
                                 stream_writer,
                                 buffered_writer<zlib_stream_writer>
                                 >;

    writer_config config;
    Writers writer;
    std::string buffer;
    writer_pimpl(writer_config config)
        : config{config}
        , writer {[&]() -> Writers {
            if (auto ptr = std::get_if<std::filesystem::path>(&config.output)) {
                if (ptr->extension() == ".fa") {
                    return file_writer{ptr->c_str()};
                } else if (ptr->extension() == ".gz") {
                    return buffered_writer{zlib_file_writer{file_writer{ptr->c_str()}}};
                }
            } else if (auto ptr = std::get_if<std::reference_wrapper<std::ostream>>(&config.output)) {
                if (!config.compressed) {
                    return stream_writer{*ptr};
                } else {
                    return buffered_writer{zlib_stream_writer{stream_writer{*ptr}}};
                }
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
writer::~writer() {
    if (pimpl) {
        std::visit([&](auto& writer) {
            writer.write({}, true);
        }, pimpl->writer);
    }
}


void writer::write(record_view record) {
    assert(pimpl);
    std::visit([&](auto& writer) {
        auto const& config = pimpl->config;
        auto& buffer = pimpl->buffer;
        buffer.clear();
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
        writer.write(buffer, false);
    }, pimpl->writer);
}

}
