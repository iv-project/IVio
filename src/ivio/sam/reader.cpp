// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include "../detail/buffered_reader.h"
#include "../detail/file_reader.h"
#include "../detail/mmap_reader.h"
#include "../detail/stream_reader.h"
#include "../detail/utilities.h"
#include "../detail/zlib_file_reader.h"
#include "reader.h"

#include <cassert>
#include <functional>
#include <optional>
#include <ranges>

namespace ivio {

template <>
struct reader_base<sam::reader>::pimpl {
    VarBufferedReader ureader;
    size_t lastUsed{};

    std::vector<std::string> header;

    pimpl(std::filesystem::path file)
        : ureader {[&]() -> VarBufferedReader {
            return mmap_reader{file};
        }()}
    {}
    pimpl(std::istream& file)
        : ureader {[&]() -> VarBufferedReader {
            return stream_reader{file};
        }()}
    {}

    bool readHeaderLine() {
        auto [buffer, size] = ureader.read(1);
        if (size >= 1 and buffer[0] == '@') {
            auto end = ureader.readUntil('\n', 0);
            if (ureader.eof(end)) throw std::runtime_error{"invalid sam header"};
            header.emplace_back(ureader.string_view(0, end));
            ureader.dropUntil(end+1);
            return true;
        }
        return false;
    }

    void readHeader() {
        while (readHeaderLine()) {} // read complete header
    }
};
}

//!WORKAROUND clang crashes if this is a member function of pimpl, see https://github.com/llvm/llvm-project/issues/61159
template <size_t ct, char sep>
static auto readLine(ivio::reader_base<ivio::sam::reader>::pimpl& self) -> std::optional<std::array<std::string_view, ct>> {
    auto res = std::array<std::string_view, ct>{};
    size_t start{};
    for (size_t i{}; i < ct-1; ++i) {
        auto end = self.ureader.readUntil(sep, start);
        if (self.ureader.eof(end)) return std::nullopt;
        res[i] = self.ureader.string_view(start, end);
        start = end+1;
    }
    auto end = self.ureader.readUntil('\n', start);
    if (self.ureader.eof(end)) return std::nullopt;
    res.back() = self.ureader.string_view(start, end);
    self.lastUsed = end;
    if (!self.ureader.eof(self.lastUsed)) self.lastUsed += 1;
    return res;
}


namespace ivio::sam {

reader::reader(config const& config_)
    : reader_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p);
    }, config_.input)}
{
    pimpl_->readHeader();
    header = std::move(pimpl_->header);
}


reader::~reader() {
    close();
}

auto reader::next() -> std::optional<record_view> {
    assert(pimpl_);

    auto& ureader  = pimpl_->ureader;
    auto& lastUsed = pimpl_->lastUsed;

    if (ureader.eof(lastUsed)) return std::nullopt;
    ureader.dropUntil(lastUsed);

    auto res = readLine<12, '\t'>(*pimpl_);
    if (!res) return std::nullopt;

    auto [qname, flag, rname, pos, mapq, cigar, rnext, pnext, tlen, seq, qual, tags] = *res;

    return record_view {.qname = qname,
                        .flag  = detail::convertTo<int32_t>(flag),
                        .rname = rname,
                        .pos   = detail::convertTo<int32_t>(pos),
                        .mapq  = detail::convertTo<int32_t>(mapq),
                        .cigar = cigar,
                        .rnext = rnext,
                        .pnext = detail::convertTo<int32_t>(pnext),
                        .tlen  = detail::convertTo<int32_t>(tlen),
                        .seq   = seq,
                        .qual  = qual,
                        .tags  = tags,
                      };
}

void reader::close() {
    pimpl_.reset();
}

static_assert(record_reader_c<reader>);

}
