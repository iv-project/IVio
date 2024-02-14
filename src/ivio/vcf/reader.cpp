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
struct reader_base<vcf::reader>::pimpl {
    VarBufferedReader ureader;
    size_t lastUsed{};

    std::vector<std::tuple<std::string, std::string>> header;
    std::vector<std::string> genotypes;

    pimpl(std::filesystem::path file)
        : ureader {[&]() -> VarBufferedReader {
            auto reader = mmap_reader{file};
            auto [buffer, len] = reader.read(2);
            if (zlib_reader::isGZipHeader({buffer, len})) {
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
            if (zlib_reader::isGZipHeader({buffer.data(), len})) {
                return zlib_reader{std::move(reader)};
            }
            return reader;
        }()}
    {}

    bool readHeaderLine() {
        auto [buffer, size] = ureader.read(2);
        if (size >= 2 and buffer[0] == '#' and buffer[1] == '#') {
            auto start = 2;
            auto mid = ureader.readUntil('=', start);
            if (ureader.eof(mid)) return false;
            auto end = ureader.readUntil('\n', mid+1);
            header.emplace_back(ureader.string_view(start, mid), ureader.string_view(mid+1, end));
            if (ureader.eof(end)) return false;
            ureader.dropUntil(end+1);
            return true;
        }
        return false;
    }

    void readHeader() {
        while (readHeaderLine()) {}
        auto [buffer, size] = ureader.read(1);
        if (size >= 1 and buffer[0] == '#') {
            auto start = 1;
            auto end = ureader.readUntil('\n', start);
            auto tableHeader = ureader.string_view(start, end);
//!WORKAROUND clang15's split_view is not working
#if __clang_major__ == 15
        {
            size_t start = 0;
            size_t pos = 0;
            while ((pos = tableHeader.find('\t', start)) != std::string::npos) {
                genotypes.emplace_back(tableHeader.begin() + start, tableHeader.begin() + pos);
                start = pos+1;
            }
            genotypes.emplace_back(tableHeader.begin() + start, tableHeader.size() - start);
        }
#else
        for (auto v : std::views::split(tableHeader, '\t')) {
    #if __GNUC__ == 11 // !WORKAROUND for gcc11
                auto cv = std::ranges::common_view{v};
                genotypes.emplace_back(cv.begin(), cv.end());
    #else
                genotypes.emplace_back(v.begin(), v.end());
    #endif
        }
#endif
            if (genotypes.size() < 9) {
                throw std::runtime_error("Header description line is invalid");
            }
            genotypes.erase(begin(genotypes), begin(genotypes)+9);
            ureader.dropUntil(end);
            if (!ureader.eof(end)) ureader.dropUntil(1);
        }
    }
};
}
//!WORKAROUND clang crashes if this is a member function of pimpl, see https://github.com/llvm/llvm-project/issues/61159
template <size_t ct, char sep>
static auto readLine(ivio::reader_base<ivio::vcf::reader>::pimpl& self) -> std::optional<std::array<std::string_view, ct>> {
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

namespace ivio::vcf {

reader::reader(config const& config_)
    : reader_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p);
    }, config_.input)}
{
    pimpl_->readHeader();
    header_.table     = std::move(pimpl_->header);
    header_.genotypes = std::move(pimpl_->genotypes);
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

    auto res = readLine<10, '\t'>(*pimpl_);
    if (!res) return std::nullopt;

    auto [chrom, pos, id, ref, alts, qual, filters, infos, formats, samples] = *res;

    return record_view {
        .chrom   = chrom,
        .pos     = detail::convertTo<int32_t>(pos),
        .id      = id,
        .ref     = ref,
        .alts    = alts,
        .qual    = detail::convertTo<float>(qual),
        .filters = filters,
        .infos   = infos,
        .formats = formats,
        .samples = samples,
    };
}

void reader::close() {
    pimpl_.reset();
}

static_assert(record_reader_c<reader>);

}
