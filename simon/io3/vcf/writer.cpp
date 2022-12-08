#include "writer.h"

#include "../buffered_writer.h"
#include "../file_writer.h"
#include "../stream_writer.h"
#include "../zlib_file_writer.h"

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

    auto const& [chrom, pos, id, ref, alt, qual, filters, infos, format, samples] = record;
    auto ss = std::string{};
    ss += chrom; ss += '\t';
    ss += std::to_string(pos); ss += '\t';
    ss += id; ss += '\t';
    ss += ref; ss += '\t';

    if (!alt.empty()) {
        ss += alt[0];
        for (size_t i{1}; i < alt.size(); ++i) {
            ss += ',';
            ss += alt[i];
        }
    }
    ss += '\t';

    if (qual) {
        auto oldSize = ss.size();
        ss.resize(oldSize + 256); // can only convert floats that fit into 256characters
        auto [ptr, ec] = std::to_chars(ss.data() + oldSize, ss.data() + ss.size(), *qual);
        if (ec == std::errc()) {
            ss.resize(ptr - ss.data());
        } else {
            // Something didn't work, fall back to slow std::stringstream implementation
            ss.resize(oldSize);
            auto str = std::stringstream{};
            str << *qual;
            ss += str.str();
        }
    } else {
        ss += '*';
    }

    ss += '\t';

    auto join = [&](auto const& vec, char d) {
        if (!vec.empty()) {
            ss += vec[0];
            for (size_t i{1}; i < vec.size(); ++i) {
                ss += d; ss += vec[i];
            }
        } else {
            return false;
        }
        ss += '\t';
        return true;
    };

    if (!join(filters, ';')) ss += ".\n";
    join(infos, ';');
    ss += format; ss += '\t';

    if (!samples.empty()) {
        ss += samples[0];
        for (size_t i{1}; i < samples.size(); ++i) {
            ss += ' '; ss += samples[i];
        }
    }
    ss += '\n';


    std::visit([&](auto& writer) {
       writer.write(ss, false);
    }, pimpl->writer);
}

}
