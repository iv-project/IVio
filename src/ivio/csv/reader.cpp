// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include "../detail/buffered_reader.h"
#include "../detail/file_reader.h"
#include "../detail/mmap_reader.h"
#include "../detail/stream_reader.h"
#include "../detail/zlib_file_reader.h"
#include "reader.h"

static_assert(std::ranges::range<ivio::csv::reader>, "reader must be a range (unittest)");
static_assert(ivio::record_reader_c<ivio::csv::reader>, "must fulfill the record_reader concept (unittest)");
static_assert(ivio::Seekable<ivio::csv::reader>, "must fulfill the Seekable concept (unittest)");

namespace ivio {

template <>
struct reader_base<csv::reader>::pimpl {
    VarBufferedReader ureader;
    char delimiter;
    size_t lastUsed{};
    std::vector<std::string> entries;

    pimpl(std::filesystem::path file, char _delimiter)
        : ureader {[&]() -> VarBufferedReader {
            auto reader = mmap_reader{file}; // create a reader and peak into the file
            auto [buffer, len] = reader.read(2);
            if (zlib_reader::isGZipHeader({buffer, len})) {
                return zlib_reader{std::move(reader)};
            }
            return reader;
        }()}
        , delimiter{_delimiter}
    {}
    pimpl(std::istream& file, char _delimiter)
        : ureader {[&]() -> VarBufferedReader {
            auto reader = stream_reader{file};
            auto buffer = std::array<char, 2>{};
            auto len = reader.read(buffer);
            reader.seek(0);
            if (zlib_reader::isGZipHeader({buffer.data(), len})) {
                return zlib_reader{std::move(reader)};
            }
            return reader;
        }()}
        , delimiter{_delimiter}
    {}
};
}

namespace ivio::csv {

reader::reader(config const& config_)
    : reader_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p, config_.delimiter);
    }, config_.input)}
{}

reader::~reader() = default;

auto reader::next() -> std::optional<record_view> {
    assert(pimpl_);

    auto& ureader   = pimpl_->ureader;
    auto& lastUsed  = pimpl_->lastUsed;
    auto& entries   = pimpl_->entries;
    auto& delimiter = pimpl_->delimiter;

    ureader.dropUntil(lastUsed);
    auto lineEnd = ureader.readUntil('\n', 0);
    if (ureader.eof(lineEnd)) return std::nullopt;

    bool lfcrEncoding = false;
    if (lineEnd > 0) {
        if (ureader.string_view(lineEnd-1, lineEnd)[0] == '\r') {
            lfcrEncoding = true;
            lineEnd -= 1;
        }
    }
    auto line = ureader.string_view(0, lineEnd);
    entries.clear();
    while (line.size()) {
        auto pos = line.find(delimiter);
        if (pos != std::string::npos) {
            auto entry = line.substr(0, pos);
            entries.emplace_back(entry);
            line = line.substr(pos+1);
        } else {
            entries.emplace_back(line);
            break;
        }
    }

    lastUsed = lineEnd+1;
    if (lfcrEncoding) lastUsed += 1;

    return record_view {
        .entries = entries,
    };
}

void reader::close() {
    pimpl_.reset();
}

auto reader::tell() const -> size_t {
    assert(pimpl_);

    auto& ureader  = pimpl_->ureader;
    return ureader.tell();
}

void reader::seek(size_t offset) {
    assert(pimpl_);

    auto& ureader  = pimpl_->ureader;

    ureader.seek(offset);
    ureader.dropUntil(0);
    pimpl_->lastUsed = 0;
}

}
