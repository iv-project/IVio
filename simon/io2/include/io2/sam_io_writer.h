#pragma once

#include "alphabet_seqan223.h"
#include "utils.h"

#include <filesystem>
#include <seqan/bam_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <string_view>


namespace io2 {

namespace sam_io {

inline auto toSeqan2Alphabet(std::ranges::range auto rng) {
    using AlphabetS3 = std::decay_t<decltype(*rng.begin())>;
    seqan::String<detail::AlphabetAdaptor<AlphabetS3>> v;
    resize(v, size(rng), seqan::Exact());
    std::ranges::copy(rng | std::views::transform([](auto c) {
        auto t = detail::AlphabetAdaptor<AlphabetS3>{};
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




template <typename AlphabetS3 = seqan3::dna5>
struct writer {
    /** Wrapper to allow path and stream inputs
     */
    struct Output {
        std::string path;
        seqan::BamFileOut fileOut{seqan::toCString(path)};

        Output(char const* _path)
            : path{_path}
        {}
        Output(std::string const& _path)
            : path{_path}
        {}
        Output(std::filesystem::path const& _path)
            : path{_path.string()}
        {}
    };

    // configurable from the outside
    Output output;
    [[no_unique_address]] detail::empty_class<AlphabetS3> alphabet{};


    template <typename T>
    struct view {
        T* v{};
        size_t size{};

        view() = default;
        view(view const&) = default;
        view(view&&) = delete;

        template <typename rng_t>
            requires range_over<rng_t, T>
                and std::ranges::contiguous_range<rng_t>
                and std::ranges::sized_range<rng_t>
        view(rng_t s)
            : v{&*s.begin()}
            , size{s.size()}
        {}

        friend auto begin(view& _view) {
            return _view.v;
        }
        friend auto end(view const& _view) {
            return _view.v + _view.size;
        }
        friend auto size(view const& _view) {
            return _view.size;
        }
        auto begin()       { return v; }
        auto end()         { return v+size; }
        auto begin() const { return v; }
        auto end()   const { return v+size; }


    };

    struct record {
        view<char>       id;
        view<AlphabetS3> seq;
    };

    void write(record _record) {
        seqan::BamAlignmentRecord r;
        r.qName = toSeqan2(_record.id);
        r.seq   = toSeqan2Alphabet(_record.seq);
        writeRecord(output.fileOut, r);
    }

    void emplace_back(range_over<char> auto const& id, range_over<AlphabetS3> auto const& seq) {
        seqan::BamAlignmentRecord record;
        record.qName = id;
        record.seq   = toSeqan2Alphabet(seq);
        writeRecord(output.fileOut, record);
    }
};

}
}
