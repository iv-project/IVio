#include "vcf_writer.h"

#include "buffered_writer.h"
#include "file_writer.h"
#include "stream_writer.h"
#include "zlib_file_writer.h"

#include <cassert>

namespace io3::vcf {

struct writer_pimpl {
    using Writers = std::variant<file_writer/*,
                                 buffered_writer<zlib_file_writer>,
                                 stream_writer,
                                 buffered_writer<zlib_stream_writer>*/
                                 >;

    writer_config config;
    Writers writer;
    writer_pimpl(writer_config config)
        : config{config}
        , writer {[&]() -> Writers {
            if (auto ptr = std::get_if<std::filesystem::path>(&config.output)) {
                return file_writer{ptr->c_str()};
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

    auto const& [chrom, pos, id, ref, alt, qual, filters, info, format, samples] = record;
    auto ss = std::stringstream{};
    ss << chrom    << '\t'
       << pos      << '\t'
       << id       << '\t'
       << ref      << '\t';
    if (!alt.empty()) {
        ss << alt[0];
        for (size_t i{1}; i < alt.size(); ++i) {
            ss << ',' << alt[i];
        }
    }
    ss << '\t'
       << qual     << '\t';
    if (!filters.empty()) {
        ss << filters[0];
        for (size_t i{1}; i < filters.size(); ++i) {
            ss << ';' << filters[i];
        }
    } else {
        ss << '.';
    }
    ss << '\t'
       << info     << '\t'
       << format   << '\t';

    if (!samples.empty()) {
        ss << samples[0];
        for (size_t i{1}; i < samples.size(); ++i) {
            ss << ' ' << samples[i];
        }
    }
    ss << '\n';
    std::visit([&](auto& writer) {
        auto str = ss.str();
        writer.write(str, true);
    }, pimpl->writer);
}

}
