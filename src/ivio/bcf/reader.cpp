#include "reader.h"

#include "../bgzf_reader.h"
#include "../bgzf_mt_reader.h"
#include "../buffered_reader.h"
#include "../file_reader.h"
#include "../mmap_reader.h"
#include "../stream_reader.h"
#include "../zlib_file_reader.h"
#include "../zlib_mmap2_reader.h"
#include "../zlib_ng_file_reader.h"

#include <cassert>
#include <charconv>
#include <functional>
#include <optional>
#include <ranges>

namespace ivio {

template <>
struct reader_base<bcf::reader>::pimpl {
    VarBufferedReader ureader;
    size_t lastUsed{};

    std::vector<std::tuple<std::string, std::string>> header;
    std::vector<std::string> genotypes;

    struct {
        std::vector<int32_t> filters;
    } storage;

    pimpl(std::filesystem::path file)
        : ureader {[&]() -> VarBufferedReader {
            return bgzf_mt_reader{mmap_reader{file}};
        }()}
    {}
    pimpl(std::istream& file)
        : ureader {[&]() -> VarBufferedReader {
            return bgzf_mt_reader{stream_reader{file}};
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
        auto [ptr, size] = ureader.read(9);

        if (size < 9) throw "something went wrong reading bcf file (1)";

        size_t txt_len = ivio::bgzfUnpack<uint32_t>(ptr + 5);
        ureader.dropUntil(9);
        std::tie(ptr, size) = ureader.read(txt_len); // read complete header

        while (readHeaderLine()) {};

        size_t s{};
        for (auto [key, value] : header) {
            s += key.size() + value.size() + 4;
        }

        std::tie(ptr, size) = ureader.read(txt_len - s);
        if (size < 1 or ptr[0] != '#') {
            throw std::runtime_error("faulty bcf header");
        }

        auto tableHeader = std::string_view{ptr, txt_len-s};
        for (auto v : std::views::split(tableHeader, '\t')) {
            genotypes.emplace_back(v.begin(), v.end());
        }
        if (genotypes.size() < 9) {
            throw std::runtime_error("Header description line is invalid");
        }
        genotypes.erase(begin(genotypes), begin(genotypes)+9);
        ureader.dropUntil(txt_len-s);
    }


    auto next() -> std::optional<bcf::record_view> {
        ureader.dropUntil(lastUsed);
        auto [ptr, size] = ureader.read(8);
        if (size == 0) return std::nullopt;
        if (size < 8) throw "something went wrong reading bcf file (2)";

        auto l_shared = ivio::bgzfUnpack<uint32_t>(ptr+0);
        auto l_indiv  = ivio::bgzfUnpack<uint32_t>(ptr+4);
        auto flen = l_shared + l_indiv + 8;
        std::tie(ptr, size) = ureader.read(flen);
        if (size < flen) throw "something went wrong reading bcf file (3)";
        if (size < 32+3) throw "something went worng reading bcf file (4)";
        auto chromId  = ivio::bgzfUnpack<int32_t>(ptr + 8);
        auto pos      = ivio::bgzfUnpack<int32_t>(ptr + 12);
        //auto rlen     = ivio::bgzfUnpack<int32_t>(ptr + 16);
        auto qual     = [&]() -> std::optional<float> {
            auto q     = ivio::bgzfUnpack<float>(ptr + 20);
            if (q == 0b0111'1111'1000'0000'0000'0000'0001) return std::nullopt;
            return q;
        }();
        //auto n_info   = ivio::bgzfUnpack<int16_t>(ptr + 24);
        auto n_allele = ivio::bgzfUnpack<int16_t>(ptr + 26);
        //auto n_sample = ivio::bgzfUnpack<int32_t>(ptr + 28) & 0x00ffffff;
        //auto n_fmt    = ivio::bgzfUnpack<uint8_t>(ptr + 31);

        auto readInt = [&](size_t o) -> std::tuple<int32_t, size_t> {
            auto v = ivio::bgzfUnpack<uint8_t>(ptr + o);
            auto t = v & 0x0f;
            if (t == 1) {
                return {ivio::bgzfUnpack<int8_t>(ptr + o + 1), o+2};
            } else if (t == 2) {
                return {ivio::bgzfUnpack<int16_t>(ptr + o + 1), o+3};
            } else if (t == 3) {
                return {ivio::bgzfUnpack<int32_t>(ptr + o + 1), o+5};
            } else {
                throw "BCF error, expected an int";
            }
        };
        auto readString = [&](size_t o) -> std::tuple<std::string_view, size_t> {
            auto v = ivio::bgzfUnpack<uint8_t>(ptr + o);
            //auto t = v & 0x0f;
            auto l = v >> 4;
            if (l == 15) {
                auto [i, o2] = readInt(o+1);
                return {{ptr+o2, ptr+o2+i}, o2+i};
            }
            return {{ptr+o+1, ptr+o+1+l}, o+1+l};
        };

        auto readVector = [&](size_t o) -> std::tuple<std::vector<int32_t>, size_t> {
            auto v = ivio::bgzfUnpack<uint8_t>(ptr + o);
            //auto t = v & 0x0f;
            auto l = v >> 4;
            if (l == 15) {
                auto [i, o2] = readInt(o+1);
                l = i;
                o = o2;
            }
            auto res = std::vector<int32_t>{};
            res.resize(l);
            for (auto& v : res) {
                std::tie(v, o) = readInt(o);
            }
            return {res, o};
        };

        auto [id, o2] = readString(32);
        auto [ref, o3] = readString(o2);

        if (chromId < 0) {
            throw "chromId is invalid, negative values not allowed";
        }
        /*if (contigMap.size() <= size_t(chromId)) {
            throw "chromId " + std::to_string(chromId) + " is missing in the header";
        }*/

        //storage.alts.clear();
        if (n_allele < 0) {
            throw "n_allele is negative, not allowed";
        }
        auto beginAlts = reinterpret_cast<uint8_t const*>(ptr + o3);
        for (size_t i{1}; i < size_t(n_allele); ++i) {
            auto [alt, o4] = readString(o3);
            o3 = o4;
        }
        auto endAlts = reinterpret_cast<uint8_t const*>(ptr + o3);
        auto alts = std::span{beginAlts, endAlts};


        auto [filters, o5] = readVector(o3);
        storage.filters.clear();
        for (auto f : filters) {
            storage.filters.emplace_back(f);
        }

        auto info = [&]() {
            assert(o5 <= l_shared+8);

            //auto p1 = o5;
            //auto p2 = l_shared+8;
            return std::string_view{ptr + o5, ptr + l_shared+8};
        }();

        lastUsed = l_shared + l_indiv + 8;


        return bcf::record_view {
            .chromId  = chromId,
            .pos      = pos,
            .id       = id,
            .ref      = ref,
            .n_allele = n_allele,
            .alts     = alts,
            .qual     = qual,
            .filters  = storage.filters,
            .info     = info,
            .format   = ureader.string_view(0, 0), //!TODO
            .samples  = ureader.string_view(0, 0), //!TODO
        };
    }
};
}

namespace ivio::bcf {

reader::reader(config const& config_)
    : reader_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p);
    }, config_.input)}
{
    pimpl_->readHeader();
}

reader::~reader() = default;

auto reader::next() -> std::optional<record_view> {
    assert(pimpl_);
    return pimpl_->next();
}

static_assert(record_reader_c<reader>);

}
