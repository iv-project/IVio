#include "writer.h"

#include "../buffered_writer.h"
#include "../file_writer.h"
#include "../stream_writer.h"
#include "../zlib_file_writer.h"

#include <cassert>
#include <charconv>

namespace ivio::vcf {

struct writer_pimpl {
    using Writers = std::variant<file_writer/*,
                                 buffered_writer<zlib_file_writer>,
                                 stream_writer,
                                 buffered_writer<zlib_stream_writer>*/
                                 >;

    writer_config config;
    Writers writer;

    bool finishedHeader{false};
    std::vector<std::string> genotype;

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
    auto ss = std::string{
        "##fileformat=VCFv4.3\n"
    };

    std::visit([&](auto& writer) {
       writer.write(ss, false);
    }, pimpl->writer);
}

writer::~writer() {
    if (pimpl) {
        std::visit([&](auto& writer) {
            writer.write({}, true);
        }, pimpl->writer);
    }
}

void writer::writeHeader(std::string_view key, std::string_view value) {
    assert(pimpl);
    if (pimpl->finishedHeader) {
        throw std::runtime_error("vcf header can't be changed after a record was written");
    }
    if (key == "fileformat") { // ignore request to write file format
        return;
    }

    auto ss = std::string{};
    ss.reserve(4 + key.size() + value.size());
    ss = "##";
    ss += key;
    ss += '=';
    ss += value;
    ss += '\n';
    std::visit([&](auto& writer) {
       writer.write(ss, false);
    }, pimpl->writer);
}

void writer::addGenotype(std::string genotype) {
    assert(pimpl);
    pimpl->genotype.emplace_back(std::move(genotype));
}


void writer::write(record_view record) {
    assert(pimpl);

    if (not pimpl->finishedHeader) {
        auto ss = std::string{"#CHROM	POS	ID	REF	ALT	QUAL	FILTER	INFO	FORMAT"};
        for (auto const& s : pimpl->genotype) {
            ss += '\t' + s;
        }
        ss += '\n';
        std::visit([&](auto& writer) {
            writer.write(ss, false);
        }, pimpl->writer);

        pimpl->finishedHeader = true;
    }

    auto const& [chrom, pos, id, ref, alts, qual, filters, infos, formats, samples] = record;
    auto ss = std::string{};
    ss.reserve(chrom.size() + id.size() + ref.size() + alts.size() + filters.size() + infos.size() + formats.size() + samples.size()
                + 100); // some guessing....100 is completly random
    ss += chrom; ss += '\t';
    ss += std::to_string(pos); ss += '\t';
    ss += id; ss += '\t';
    ss += ref; ss += '\t';
    ss += alts; ss += '\t';

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

    ss += filters; ss += '\t';
    ss += infos; ss += '\t';
    ss += formats; ss += '\t';
    ss += samples; ss += '\n';


    std::visit([&](auto& writer) {
       writer.write(ss, false);
    }, pimpl->writer);
}

}
