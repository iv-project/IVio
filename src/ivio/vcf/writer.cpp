// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include "writer.h"

#include "../detail/buffered_writer.h"
#include "../detail/file_writer.h"
#include "../detail/stream_writer.h"
#include "../detail/zlib_file_writer.h"

#include <cassert>
#include <charconv>
#include <sstream>

template <>
struct ivio::writer_base<ivio::vcf::writer>::pimpl {
    using Writers = std::variant<ivio::file_writer,
                                 ivio::buffered_writer<ivio::zlib_file_writer>,
                                 ivio::stream_writer,
                                 ivio::buffered_writer<ivio::zlib_stream_writer>
                                 >;

    ivio::vcf::writer::config config;
    Writers writer;

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
    // write header
    std::visit([&](auto& writer) {
        writer.write(std::string{"##fileformat=VCFv4.3\n"}); // write file format
        // write leading table
        auto ss = std::string{};
        for (auto [key, value] : config_.header.table) {
            if (key == "fileformat") continue; // ignore request to write file format
            ss = "##";
            ss += key;
            ss += '=';
            ss += value;
            ss += '\n';
            writer.write(ss);
        }
        // write heading of the body
        {
            auto _ss = std::string{"#CHROM	POS	ID	REF	ALT	QUAL	FILTER	INFO	FORMAT"};
            for (auto const& s : config_.header.genotypes) {
                _ss += '\t' + s;
            }
            _ss += '\n';
            writer.write(_ss);
        }
    }, pimpl_->writer);
}

writer::~writer() = default;

void writer::write(record_view record) {
    assert(pimpl_);

    auto const& [chrom, pos, id, ref, alts, qual, filters, infos, formats, samples] = record;
    static thread_local auto ss = std::string{};
    ss = chrom; ss += '\t';
    ss += std::to_string(pos); ss += '\t';
    ss += id; ss += '\t';
    ss += ref; ss += '\t';
    ss += alts; ss += '\t';

    if (qual) {
        auto oldSize = ss.size();
        //!WORKAROUND std::from_chars for float/double is missing from libc++
        // libc++ does not define __cpp_lib_to_chars
        #if !defined(_LIBCPP_VERSION) || defined(__cpp_lib_to_chars)
        ss.resize(oldSize + 256); // can only convert floats that fit into 256characters
        auto [ptr, ec] = std::to_chars(ss.data() + oldSize, ss.data() + ss.size(), *qual);
        if (ec == std::errc()) {
            ss.resize(ptr - ss.data());
        } else
        #endif
        {
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
       writer.write(ss);
    }, pimpl_->writer);
}

void writer::close() {
    pimpl_.reset();
}

static_assert(record_writer_c<writer>);

}
