#pragma once

#include "alphabet_seqan223.h"
#include "utils.h"

#include <filesystem>
#include <seqan/bam_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <string_view>


namespace io2::sam_io::detail {

inline auto toSeqan2Alphabet(std::ranges::range auto rng) {
    using AlphabetS3 = std::decay_t<decltype(*rng.begin())>;
    seqan::String<io2::detail::AlphabetAdaptor<AlphabetS3>> v;
    resize(v, size(rng), seqan::Exact());
    std::ranges::copy(rng | std::views::transform([](auto c) {
        auto t = io2::detail::AlphabetAdaptor<AlphabetS3>{};
        t.value = c.to_rank();
        return t;
    }), begin(v));
    return v;
}
inline auto toSeqan2Qualities(std::ranges::range auto rng) {
    return toSeqan2Alphabet(rng);
}


inline auto toSeqan2Cigar(std::ranges::range auto rng) {
    seqan::String<seqan::CigarElement<>> v;
    resize(v, size(rng), seqan::Exact());
    std::ranges::copy(rng | std::views::transform([](auto c) {
        auto t = seqan::CigarElement{};
        t.operation = get<1>(c).to_char();
        t.count     = get<0>(c);
        return t;
    }), begin(v));
    return v;
}


inline auto toSeqan2(std::ranges::range auto rng) {
    seqan::String<char> v;
    resize(v, size(rng));
    std::ranges::copy(rng, begin(v));
    return v;
}

}


namespace io2::sam_io {

template <typename AlphabetS3 = seqan3::dna5,
          typename QualitiesS3 = seqan3::phred42>
struct writer {
    /** Wrapper to allow path and stream inputs
     */
    struct Output {
        seqan::BamFileOut fileOut;

        Output(char const* _path)
            : fileOut{_path}
        {}
        Output(std::string const& _path)
            : Output(_path.c_str())
        {}
        Output(std::filesystem::path const& _path)
            : Output(_path.c_str())
        {}
    };

    // configurable from the outside
    Output output;
    [[no_unique_address]] io2::detail::empty_class<AlphabetS3>  alphabet{};
    [[no_unique_address]] io2::detail::empty_class<QualitiesS3> qualities{};


    template <typename T>
    struct contigous_range {
        T const* v{};
        size_t size{};

        contigous_range() = default;
        contigous_range(contigous_range const&) = default;
        contigous_range(contigous_range&&) = delete;

        template <typename rng_t>
            requires range_over<rng_t, T>
                and std::ranges::contiguous_range<rng_t>
                and std::ranges::sized_range<rng_t>
        contigous_range(rng_t const& s)
            : v{&*s.begin()}
            , size{s.size()}
        {}

        friend auto begin(contigous_range& _contigous_range) {
            return _contigous_range.v;
        }
        friend auto end(contigous_range const& _contigous_range) {
            return _contigous_range.v + _contigous_range.size;
        }
        friend auto size(contigous_range const& _contigous_range) {
            return _contigous_range.size;
        }

        auto begin() const { return v; }
        auto end()   const { return v+size; }
    };

    struct record {
        contigous_range<char>          id;
        uint16_t                       flag{};
        std::optional<int32_t>         rID;
        std::optional<int32_t>         beginPos;
        uint8_t                        mapQ{};
        uint16_t                       bin{};
        contigous_range<seqan3::cigar> cigar;
        int32_t                        rNextId{};
        int32_t                        pNext{};
        int32_t                        tLen{};
        contigous_range<AlphabetS3>    seq;
        contigous_range<QualitiesS3>   qual;
        contigous_range<char>          tags;
    };

    void write(record _record) {
        seqan::BamAlignmentRecord r;
        r.qName    = detail::toSeqan2(_record.id);
        r.flag     = _record.flag;
        r.rID      = _record.rID.value_or(seqan::BamAlignmentRecord::INVALID_REFID);
        r.beginPos = _record.beginPos.value_or(seqan::BamAlignmentRecord::INVALID_POS);
        r.mapQ     = _record.mapQ;
        r.bin      = _record.bin;
        r.cigar    = detail::toSeqan2Cigar(_record.cigar);
        r.rNextId  = _record.rNextId;
        r.pNext    = _record.pNext;
        r.tLen     = _record.tLen;
        r.seq      = detail::toSeqan2Alphabet(_record.seq);
        r.qual     = detail::toSeqan2Qualities(_record.qual);
        r.tags     = detail::toSeqan2(_record.tags);

        writeRecord(output.fileOut, r);
    }

    void emplace_back(range_over<char> auto const& id, range_over<AlphabetS3> auto const& seq) {
        seqan::BamAlignmentRecord record;
        record.qName = id;
        record.seq   = detail::toSeqan2Alphabet(seq);
        writeRecord(output.fileOut, record);
    }
};

}
