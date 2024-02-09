// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include "../detail/buffered_reader.h"
#include "../detail/file_reader.h"
#include "../detail/mmap_reader.h"
#include "../detail/stream_reader.h"
#include "../detail/zlib_file_reader.h"
#include "reader.h"

static_assert(std::ranges::range<ivio::fasta::reader>, "reader must be a range (unittest)");
static_assert(ivio::record_reader_c<ivio::fasta::reader>, "must fulfill the record_reader concept (unittest)");
static_assert(ivio::Seekable<ivio::fasta::reader>, "must fulfill the Seekable concept (unittest)");

namespace ivio {

template <>
struct reader_base<fasta::reader>::pimpl {
    VarBufferedReader ureader;
    size_t lastUsed{};
    std::string s;

    faidx::record_view faidxView;

    pimpl(std::filesystem::path file)
        : ureader {[&]() -> VarBufferedReader {
            auto reader = mmap_reader{file}; // create a reader and peak into the file
            auto [buffer, len] = reader.read(2);
            if (zlib_reader::checkHeader({buffer, len})) {
                return zlib_reader{std::move(reader)};
            }
            return reader;
        }()}
    {}
    pimpl(std::istream& file)
        : ureader {[&]() -> VarBufferedReader {
            auto reader = stream_reader{file};
            auto buffer = std::array<char, 2>{};
            auto len = reader.read(buffer);
            reader.seek(0);
            if (zlib_reader::checkHeader({buffer.data(), len})) {
                return zlib_reader{std::move(reader)};
            }
            return reader;
        }()}
    {}
};
}

namespace ivio::fasta {

reader::reader(config const& config_)
    : reader_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p);
    }, config_.input)}
{}

reader::~reader() = default;

auto reader::next() -> std::optional<record_view> {
    assert(pimpl_);

    auto& ureader  = pimpl_->ureader;
    auto& lastUsed = pimpl_->lastUsed;
    auto& s        = pimpl_->s;

    auto startId = ureader.readUntil('>', lastUsed);
    if (ureader.eof(startId)) return std::nullopt;
    ureader.dropUntil(startId+1);

    auto endId = ureader.readUntil('\n', 0);
    if (ureader.eof(endId)) return std::nullopt;

    auto startSeq = endId+1;

    bool lfcrEncoding = false;
    if (endId > 0) {
        if (ureader.string_view(endId-1, endId)[0] == '\r') {
            lfcrEncoding = true;
            endId -= 1;
        }
    }

    pimpl_->faidxView.offset = tell() + startSeq;

    // convert into dense string representation
    size_t firstLineLength = std::numeric_limits<size_t>::max();
    s.clear();
    {
        auto s2 = startSeq;
        do {
            auto s1 = s2;
            s2 = ureader.readUntil('\n', s1);
            s += ureader.string_view(s1, s2);
            if (s.size() > 0 && s.back() == '\r') {
                s.pop_back();
            }
            firstLineLength = std::min(firstLineLength, s.size());
            if (!ureader.eof(s2)) {
                s2 += 1;
            }
        } while (!ureader.eof(s2) and ureader.string_view(s2, s2+1)[0] != '>');
        lastUsed = s2;
    }

    pimpl_->faidxView.id        = ureader.string_view(0, endId);
    pimpl_->faidxView.length    = s.size();
    pimpl_->faidxView.linebases = firstLineLength;
    pimpl_->faidxView.linewidth = firstLineLength + 1 + (lfcrEncoding?1:0);

    return record_view {
        .id  = ureader.string_view(0, endId),
        .seq = s,
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

auto reader::tell_faidx() const -> faidx::record {
    assert (pimpl_);
    return pimpl_->faidxView;
}

void reader::seek_faidx(faidx::record const& faidx) {
    if (faidx.linebases + 1 != faidx.linewidth
        && faidx.linebases + 2 != faidx.linewidth) {
        throw std::runtime_error{"FAIDX record expects an unknown line ending format"};
    }
    bool lfcrEnding = (faidx.linebases + 2 == faidx.linewidth);
    if (faidx.offset < faidx.id.size() + 2 + (lfcrEnding?1:0)) {
        throw std::runtime_error("Invalid faidx seek index, offset: " + std::to_string(faidx.offset) + ", id size: " + faidx.id);
    }

    seek(faidx.offset - faidx.id.size() - 2 - (lfcrEnding?1:0));
}

}
