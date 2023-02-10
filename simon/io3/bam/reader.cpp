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

namespace io3 {


template <>
struct reader_base<bam::reader>::pimpl {
    VarBufferedReader ureader;
    size_t lastUsed{};

    std::string headerBuffer;
    std::vector<std::string_view> header;
    std::string_view              tableHeader;
    std::unordered_map<std::string_view, std::vector<std::string_view>> headerMap;

    std::vector<std::string_view>& contigMap = headerMap["contig"];
    std::vector<std::string_view>& filterMap = headerMap["filter"];

    pimpl(std::filesystem::path file, bool, size_t threadNbr)
        : ureader {[&]() -> VarBufferedReader {
            if (threadNbr == 0) {
                return buffered_reader<1<<16>{bgzf_reader{mmap_reader{file.c_str()}}};
            }
            return bgzf_mt_reader{mmap_reader{file.c_str()}, threadNbr};
        }()}
    {}
    pimpl(std::istream& file, bool compressed, size_t threadNbr)
        : ureader {[&]() -> VarBufferedReader {
            if (!compressed) {
                return stream_reader{file};
            } else if (threadNbr == 0) {
                return buffered_reader<1<<16>{bgzf_reader{stream_reader{file}}};
            }
            return bgzf_mt_reader{stream_reader{file}, threadNbr};
        }()}
    {}



    auto parseHeaderLine(size_t pos) -> std::tuple<bool, size_t> {
        if (headerBuffer.size() >= pos + 2 && headerBuffer[pos + 0] == '#' && headerBuffer[pos + 1] == '#') {
            auto start = pos + 2;
            auto end   = headerBuffer.find('\n', start);
            if (end == std::string::npos) end = headerBuffer.size();
            header.emplace_back(headerBuffer.data()+start, headerBuffer.data()+end);
            {
                auto v = header.back();
                auto p = v.find('=');
                if (p == std::string::npos) throw "error parsing BAM Header";
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
        auto [ptr, size] = ureader.read(12);

        if (size < 12) throw "something went wrong reading bam file (1)";

        size_t l_text = io3::bgzfUnpack<uint32_t>(ptr + 4);
        ureader.read(12 + l_text); // read complete header !TODO safe header for future processing
        // !TODO copy header somewhere required
        auto n_ref = io3::bgzfUnpack<uint32_t>(ptr + 8 + l_text);
        ureader.dropUntil(12 + l_text);

        // read list of references
        for (size_t i{0}; i < n_ref; ++i) {
            auto [ptr, size] = ureader.read(9);
            if (size < 9) throw " something went wrong reading bam file (1b)";
            auto l_name = io3::bgzfUnpack<uint32_t>(ptr);
            auto [ptr2, size2] = ureader.read(8 + l_name); // !TODO this is not safe
            auto l_ref  = io3::bgzfUnpack<uint32_t>(ptr2+4+l_name);
            //!TODO read name and save it somewhere
            ureader.dropUntil(8+l_name);
        }
    }

    auto next() -> std::optional<bam::record_view> {
        ureader.dropUntil(lastUsed);
        auto [ptr, size] = ureader.read(40);
        if (size == 0) return std::nullopt;
        if (size < 40) throw "something went wrong reading bam file (2)";

        auto block_size  = io3::bgzfUnpack<uint32_t>(ptr+0);

        auto [ptr2, size2] = ureader.read(block_size+4); // read again, !TODO this could fail
        auto refID       = io3::bgzfUnpack<int32_t>(ptr2+4);
        auto pos         = io3::bgzfUnpack<int32_t>(ptr2+8);
        auto l_read_name = io3::bgzfUnpack<uint8_t>(ptr2+12);
        auto mapq        = io3::bgzfUnpack<uint8_t>(ptr2+13);
        auto bin         = io3::bgzfUnpack<uint16_t>(ptr2+14);
        auto n_cigar_op  = io3::bgzfUnpack<uint16_t>(ptr2+16);
        auto flag        = io3::bgzfUnpack<uint16_t>(ptr2+18);
        auto l_seq       = io3::bgzfUnpack<uint32_t>(ptr2+20);
        auto next_refID  = io3::bgzfUnpack<int32_t>(ptr2+24);
        auto next_pos    = io3::bgzfUnpack<int32_t>(ptr2+28);
        auto tlen        = io3::bgzfUnpack<int32_t>(ptr2+32);
        auto read_name   = ureader.string_view(36, 36+l_read_name);
        auto start_cigar = l_read_name + 36;
        auto cigar       = ureader.string_view(start_cigar, start_cigar+n_cigar_op*4);
        auto start_seq   = start_cigar+n_cigar_op*4;
        auto seq         = ureader.string_view(start_seq, start_seq + (l_seq+1)/2);
        auto start_qual  = start_seq + (l_seq+1)/2;
        auto qual        = ureader.string_view(start_qual, start_qual + l_seq);
        //!TODO List of auxiliary data (tags) is missing

        lastUsed = block_size+4;

        return bam::record_view { .refID      = refID,
                                  .pos        = pos,
                                  .mapq       = mapq,
                                  .bin        = bin,
                                  .flag       = flag,
                                  .next_refID = next_refID,
                                  .next_pos   = next_pos,
                                  .tlen       = tlen,
                                  .read_name  = read_name,
                                  .cigar      = std::span{reinterpret_cast<uint8_t const*>(cigar.data()), cigar.size()},
                                  .seq        = {std::span{reinterpret_cast<uint8_t const*>(seq.data()), seq.size()}, l_seq},
                                  .qual       = std::span{reinterpret_cast<uint8_t const*>(qual.data()), qual.size()},
                                };
    }
};
}

namespace io3::bam {

reader::reader(config const& config_)
    : reader_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p, config_.compressed, config_.threadNbr);
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
