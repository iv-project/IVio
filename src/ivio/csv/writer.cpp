// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include "../detail/buffered_writer.h"
#include "../detail/file_writer.h"
#include "../detail/stream_writer.h"
#include "../detail/zlib_file_writer.h"
#include "writer.h"

#include <cassert>

namespace ivio {

template <>
struct writer_base<csv::writer>::pimpl {
    using Writers = std::variant<file_writer,
                                 buffered_writer<zlib_file_writer>,
                                 stream_writer,
                                 buffered_writer<zlib_stream_writer>
                                 >;

    Writers writer;
    char delimiter;
    std::string buffer;
    pimpl(std::filesystem::path output, bool, char _delimiter)
        : writer {[&]() -> Writers {
            if (output.extension() == ".gz") {
                return buffered_writer{zlib_file_writer{file_writer{output}}};
            }
            return file_writer{output};
        }()}
        , delimiter{_delimiter}
    {}

    pimpl(std::ostream& output, bool compressed, char _delimiter)
        : writer {[&]() -> Writers {
            if (compressed) {
                return buffered_writer{zlib_stream_writer{stream_writer{output}}};
            }
            return stream_writer{output};
        }()}
        , delimiter{_delimiter}
    {}
};

}

namespace ivio::csv {

writer::writer(config config_)
    : writer_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p, config_.compressed, config_.delimiter);
    }, config_.output)}
{
}

writer::~writer() = default;

void writer::write(record_view record) {
    assert(pimpl_);
    std::visit([&](auto& writer) {
        auto const& delimiter = pimpl_->delimiter;
        auto& buffer = pimpl_->buffer;
        buffer.clear();
        if (!record.entries.empty()) {
            buffer = record.entries[0];
        }
        for (size_t i{1}; i < record.entries.size(); ++i) {
            buffer = buffer + delimiter + record.entries[i];
        }
        buffer += '\n';
        writer.write(buffer);
    }, pimpl_->writer);
}

void writer::close() {
    pimpl_.reset();
}

static_assert(record_writer_c<writer>);

}
