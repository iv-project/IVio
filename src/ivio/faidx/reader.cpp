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

static_assert(std::ranges::range<ivio::faidx::reader>, "reader must be a range (unittest)");
static_assert(ivio::record_reader_c<ivio::faidx::reader>, "must fulfill the record_reader concept (unittest)");

namespace ivio {

template <>
struct reader_base<faidx::reader>::pimpl {
    VarBufferedReader ureader;
    size_t lastUsed{};

    pimpl(std::filesystem::path file, bool)
        : ureader {[&]() -> VarBufferedReader {
            if (file.extension() == ".gz") {
                return zlib_reader{mmap_reader{file}};
            }
            return mmap_reader{file};
        }()}
    {}
    pimpl(std::istream& file, bool compressed)
        : ureader {[&]() -> VarBufferedReader {
            if (!compressed) {
                return stream_reader{file};
            }
            return zlib_reader{stream_reader{file}};
        }()}
    {}
};
}


//!WORKAROUND clang crashes if this is a member function of pimpl, see https://github.com/llvm/llvm-project/issues/61159
template <size_t ct, char sep>
static auto readLine(ivio::reader_base<ivio::faidx::reader>::pimpl& self) -> std::optional<std::array<std::string_view, ct>> {
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


namespace ivio::faidx {

reader::reader(config const& config_)
    : reader_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p, config_.compressed);
    }, config_.input)}
{}

reader::~reader() = default;

auto reader::next() -> std::optional<record_view> {
    assert(pimpl_);

    auto& ureader  = pimpl_->ureader;
    auto& lastUsed = pimpl_->lastUsed;

    ureader.dropUntil(lastUsed);

    auto res = readLine<5, '\t'>(*pimpl_);
    if (!res) return std::nullopt;

    auto [id, length, offset, linebases, linewidth] = *res;

    return record_view {.id = id,
                        .length    = detail::convertTo<size_t>(length),
                        .offset    = detail::convertTo<size_t>(offset),
                        .linebases = detail::convertTo<size_t>(linebases),
                        .linewidth = detail::convertTo<size_t>(linewidth),
                      };
}

void reader::close() {
    pimpl_.reset();
}

}
