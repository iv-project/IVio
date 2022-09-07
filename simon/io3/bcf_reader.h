#pragma once

#include "bgzf_reader.h"
#include "buffered_reader.h"
#include "file_reader.h"
#include "vcf_reader.h"

namespace io3 {

template <typename Reader>
struct bcf_reader {
    Reader reader;
    size_t lastUsed{};

    using record_view = vcf_reader_view_record_view;
    using iter        = vcf_reader_view_iter;

    template <typename R>
    bcf_reader(R&& r)
        : reader{std::forward<R>(r)}
    {
        readHeader();
    }


    bcf_reader(bcf_reader const&) = delete;
    bcf_reader(bcf_reader&& _other) noexcept = default;
    ~bcf_reader() = default;


    friend auto begin(bcf_reader& reader) {
        return iter{[&reader]() { return reader.next(); }};
    }
    friend auto end(bcf_reader const&) {
        return nullptr;
    }

    std::string headerBuffer;
    std::vector<std::string_view> header;
    std::string_view              tableHeader;
    std::unordered_map<std::string_view, std::vector<std::string_view>> headerMap;

    std::vector<std::string_view>& contigMap = headerMap["contig"];
    std::vector<std::string_view>& filterMap = headerMap["FILTER"];

    struct {
        std::vector<std::string_view> alts;
        std::vector<std::string_view> samples;
    } storage;


    auto parseHeaderLine(size_t pos) -> std::tuple<bool, size_t> {
        if (headerBuffer.size() >= pos + 2 && headerBuffer[pos + 0] == '#' && headerBuffer[pos + 1] == '#') {
            auto start = pos + 2;
            auto end   = headerBuffer.find('\n', start);
            if (end == std::string::npos) end = headerBuffer.size();
            header.emplace_back(headerBuffer.data()+start, headerBuffer.data()+end);
            {
                auto v = header.back();
                auto p = v.find('=');
                if (p == std::string::npos) throw "error parsing BCF Header";
                auto key   = std::string_view{v.data(), v.data()+p};
                auto value = std::string_view{v.data()+p+1, v.data()+v.size()};
                headerMap[key].push_back(value);
            }
            if (end == headerBuffer.size()) return {false, end};
            return {true, end+1};
        }
        return {false, pos};
    }

    void parseHeader() {
        size_t pos{};
        bool cont;
        do {
            std::tie(cont, pos) = parseHeaderLine(pos);
        } while (cont);

        if (pos < headerBuffer.size() and headerBuffer[pos] == '#') {
            auto start = pos + 1;
            auto end = headerBuffer.find('\n', start);
            if (end == std::string::npos) end = headerBuffer.size();
            tableHeader = {headerBuffer.data() + start, headerBuffer.data() + end};
        }
    }

    void readHeader() {
        auto [ptr, size] = reader.read(9);
        if (size < 9) throw "something went wrong reading bcf file (1)";

        size_t txt_len = io3::bgzfUnpack<uint32_t>(ptr + 5);
        reader.read(9 + txt_len); // read complete header !TODO safe header for future processing
        headerBuffer = std::string{reader.string_view(9, 9+txt_len)};
        parseHeader();
        reader.dropUntil(9 + txt_len);
    }

    auto next() -> std::optional<record_view> {
        reader.dropUntil(lastUsed);
        auto [ptr, size] = reader.read(8);
        if (size == 0) return std::nullopt;
        if (size < 8) throw "something went wrong reading bcf file (2)";

        auto l_shared = io3::bgzfUnpack<uint32_t>(ptr+0);
        auto l_indiv  = io3::bgzfUnpack<uint32_t>(ptr+4);
        auto flen = l_shared + l_indiv + 8;
        auto [ptr2, size2] = reader.read(flen);
        if (size2 < flen) throw "something went wrong reading bcf file (3)";
        if (size2 < 32+4) throw "something went worng reaing bcf file (4)";
        auto chromId  = io3::bgzfUnpack<int32_t>(ptr2 + 8);
        auto pos      = io3::bgzfUnpack<int32_t>(ptr2 + 12)+1;
        auto rlen     = io3::bgzfUnpack<int32_t>(ptr2 + 16);
//        auto qual = io3::bgzfUnpack<float>(ptr2 + 20);
        auto n_info   = io3::bgzfUnpack<int16_t>(ptr2 + 24);
        auto n_allele = io3::bgzfUnpack<int16_t>(ptr2 + 26);
        auto n_sample = io3::bgzfUnpack<int32_t>(ptr2 + 28) & 0x00ffffff;
        auto n_fmt    = io3::bgzfUnpack<uint8_t>(ptr2 + 31);

        auto readInt = [&](size_t o) -> std::tuple<int32_t, size_t> {
            auto v = io3::bgzfUnpack<uint8_t>(ptr2 + o);
            auto t = v & 0x0f;
            if (t == 1) {
                return {io3::bgzfUnpack<int8_t>(ptr2 + o + 1), o+2};
            } else if (t == 2) {
                return {io3::bgzfUnpack<int16_t>(ptr2 + o + 1), o+3};
            } else if (t == 3) {
                return {io3::bgzfUnpack<int32_t>(ptr2 + o + 1), o+5};
            } else {
                throw "BCF error, expected an int";
            }
        };
        auto readString = [&](size_t o) -> std::tuple<std::string_view, size_t> {
            auto v = io3::bgzfUnpack<uint8_t>(ptr2 + o);
            auto t = v & 0x0f;
            auto l = v >> 4;
            if (l == 15) {
                auto [i, o2] = readInt(o+1);
                return {{ptr2+o2, ptr2+o2+i}, o2+i};
            }
            return {{ptr2+o+1, ptr2+o+1+l}, o+1+l};
        };
        auto [id, o2] = readString(32);
        auto [ref, o3] = readString(o2);

        if (contigMap.size() <= chromId) {
            throw "chromId " + std::to_string(chromId) + " is missing in the header";
        }

        storage.alts.clear();
        for (size_t i{1}; i < n_allele; ++i) {
            auto [alt, o4] = readString(o3);
            storage.alts.emplace_back(alt);
            o3 = o4;
        }

        lastUsed = l_shared + l_indiv + 8;
        return record_view {
            .chrom   = contigMap[chromId],
            .pos     = pos,
            .id      = id,
            .ref     = ref,
            .alt     = storage.alts,
            .qual    = 0.,
//            .filter  = reader.string_view(0, 0),
            .info    = reader.string_view(0, 0),
            .format  = reader.string_view(0, 0),
            .samples = storage.samples,
        };
    }
};

template <typename Reader>
bcf_reader(Reader&& reader) -> bcf_reader<io3::buffered_reader<Reader, 1<<16>>;

template <reader_and_dropper_c Reader>
bcf_reader(Reader&& reader) -> bcf_reader<Reader>;

static_assert(record_reader_c<bcf_reader<io3::buffered_reader<io3::file_reader>>>);
}
