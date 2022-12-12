#include "reader.h"

#include "../bgzf_reader.h"
#include "../file_reader.h"

namespace io3::bcf {

struct reader_pimpl {
    VarBufferedReader _reader;
    size_t lastUsed{};

    std::string headerBuffer;
    std::vector<std::string_view> header;
    std::string_view              tableHeader;
    std::unordered_map<std::string_view, std::vector<std::string_view>> headerMap;

    std::vector<std::string_view>& contigMap = headerMap["contig"];
    std::vector<std::string_view>& filterMap = headerMap["filter"];

    struct {
        std::vector<std::string_view> alts;
        std::vector<std::string_view> filters;
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
        for (auto v : headerMap["FILTER"]) {
            auto pos = v.find("IDX=");
            if (pos == std::string_view::npos) throw "unexpected formating";
            auto pos2 = v.find('>', pos+4); // !TODO might also be a ","
            if (pos2 == std::string_view::npos) throw "unexpected formatting (2)";
            auto v2 = v.substr(pos+4, pos2);
            auto idx = size_t(std::stoi(std::string{v2})); //!TODO from_chars is faster
            headerMap["filter"].resize(std::max(headerMap["filter"].size(), idx+1));
            headerMap["filter"][idx] = v;
        }

    }

    void readHeader() {
        auto [ptr, size] = _reader.read(9);

        if (size < 9) throw "something went wrong reading bcf file (1)";

        size_t txt_len = io3::bgzfUnpack<uint32_t>(ptr + 5);
        _reader.read(9 + txt_len); // read complete header !TODO safe header for future processing
        headerBuffer = std::string{_reader.string_view(9, 9+txt_len)};
        parseHeader();
        _reader.dropUntil(9 + txt_len);
    }

    auto next() -> std::optional<record_view> {
        _reader.dropUntil(lastUsed);
        auto [ptr, size] = _reader.read(8);
        if (size == 0) return std::nullopt;
        if (size < 8) throw "something went wrong reading bcf file (2)";

        auto l_shared = io3::bgzfUnpack<uint32_t>(ptr+0);
        auto l_indiv  = io3::bgzfUnpack<uint32_t>(ptr+4);
        auto flen = l_shared + l_indiv + 8;
        auto [ptr2, size2] = _reader.read(flen);
        if (size2 < flen) throw "something went wrong reading bcf file (3)";
        if (size2 < 32+3) throw "something went worng reading bcf file (4)";
        auto chromId  = io3::bgzfUnpack<int32_t>(ptr2 + 8);
        auto pos      = io3::bgzfUnpack<int32_t>(ptr2 + 12)+1;
        auto rlen     = io3::bgzfUnpack<int32_t>(ptr2 + 16);
        auto qual     = [&]() -> std::optional<float> {
            auto q     = io3::bgzfUnpack<float>(ptr2 + 20);
            if (q == 0b0111'1111'1000'0000'0000'0000'0001) return std::nullopt;
            return q;
        }();
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

        auto readVector = [&](size_t o) -> std::tuple<std::vector<int32_t>, size_t> {
            auto v = io3::bgzfUnpack<uint8_t>(ptr2 + o);
            auto t = v & 0x0f;
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

        if (contigMap.size() <= chromId) {
            throw "chromId " + std::to_string(chromId) + " is missing in the header";
        }

        storage.alts.clear();
        for (size_t i{1}; i < n_allele; ++i) {
            auto [alt, o4] = readString(o3);
            storage.alts.emplace_back(alt);
            o3 = o4;
        }

        auto [filters, o5] = readVector(o3);
        storage.filters.clear();
        for (auto f : filters) {
            storage.filters.emplace_back(filterMap.at(f));
        }

        auto info = [&]() {
            assert(o5 <= l_shared+8);

            auto p1 = o5;
            auto p2 = l_shared+8;
            return std::string_view{ptr2 + o5, ptr2 + l_shared+8};
        }();

        lastUsed = l_shared + l_indiv + 8;


        return record_view {
            .chrom   = contigMap[chromId],
            .pos     = pos,
            .id      = id,
            .ref     = ref,
            .alt     = storage.alts,
            .qual    = qual,
            .filter  = storage.filters,
            .info    = info,
            .format  = _reader.string_view(0, 0),
            .samples = storage.samples,
        };
    }
};

reader::reader(VarBufferedReader r)
    : pimpl{std::make_unique<reader_pimpl>(std::move(r))}
{
    pimpl->readHeader();
}

reader::~reader() = default;
auto reader::next() -> std::optional<record_view> {
    assert(pimpl);
    return pimpl->next();
}
auto begin(reader& _reader) -> reader::iter {
    return reader::iter{_reader};
}

}
