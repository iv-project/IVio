#include "writer.h"

#include "../buffered_writer.h"
#include "../file_writer.h"
#include "../stream_writer.h"
#include "../zlib_file_writer.h"

#include <cassert>
#include <charconv>

template <>
struct ivio::writer_base<ivio::vcf::writer>::pimpl {
    using Writers = std::variant<file_writer,
                                 buffered_writer<zlib_file_writer>,
                                 stream_writer,
                                 buffered_writer<zlib_stream_writer>
                                 >;

    ivio::vcf::writer::config config;
    Writers writer;

    bool finishedHeader{false};
    std::vector<std::string> genotype;

    pimpl(std::filesystem::path output, bool)
        : writer {[&]() -> Writers {
            if (output.extension() == ".gz") {
                return buffered_writer{zlib_file_writer{file_writer{output}}};
            }
            return file_writer{output};
        }()}
    {}

    pimpl(std::ostream& output, bool compressed)
        : writer {[&]() -> Writers {
            if (compressed) {
                return buffered_writer{zlib_stream_writer{stream_writer{output}}};
            }
            return stream_writer{output};
        }()}
    {}
};

namespace ivio::vcf {

writer::writer(config config_)
    : writer_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p, config_.compressed);
    }, config_.output)}
{
    auto ss = std::string{
        "##fileformat=VCFv4.3\n"
    };

    std::visit([&](auto& writer) {
       writer.write(ss, false);
    }, pimpl_->writer);
}

writer::~writer() {
    if (pimpl_) {
        std::visit([&](auto& writer) {
            writer.write({}, true);
        }, pimpl_->writer);
    }
}

void writer::writeHeader(std::string_view key, std::string_view value) {
    assert(pimpl_);
    if (pimpl_->finishedHeader) {
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
    }, pimpl_->writer);
}

void writer::addGenotype(std::string genotype) {
    assert(pimpl_);
    pimpl_->genotype.emplace_back(std::move(genotype));
}


void writer::write(record_view record) {
    assert(pimpl_);

    if (not pimpl_->finishedHeader) {
        auto ss = std::string{"#CHROM	POS	ID	REF	ALT	QUAL	FILTER	INFO	FORMAT"};
        for (auto const& s : pimpl_->genotype) {
            ss += '\t' + s;
        }
        ss += '\n';
        std::visit([&](auto& writer) {
            writer.write(ss, false);
        }, pimpl_->writer);

        pimpl_->finishedHeader = true;
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
    }, pimpl_->writer);
}

}
