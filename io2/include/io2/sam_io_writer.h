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

inline auto toSeqan2(std::ranges::range auto rng) {
    seqan::String<char> v;
    resize(v, size(rng));
    std::ranges::copy(rng, begin(v));
    return v;
}

}


namespace io2::sam_io {

template <typename AlphabetS3 = seqan3::dna5>
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
    [[no_unique_address]] io2::detail::empty_class<AlphabetS3> alphabet{};


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
        contigous_range<char>       id;
        contigous_range<AlphabetS3> seq;
    };

    void write(record _record) {
        seqan::BamAlignmentRecord r;
        r.qName = detail::toSeqan2(_record.id);
        r.seq   = detail::toSeqan2Alphabet(_record.seq);
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
