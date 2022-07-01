#pragma once

#include "alphabet_seqan223.h"

#include <filesystem>
#include <optional>
#include <ranges>
#include <seqan/seq_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <string_view>


namespace io2 {

namespace seq_io {

inline auto toSeqan2(std::ranges::range auto rng) {
    using AlphabetS3 = std::decay_t<decltype(*rng.begin())>;
    seqan::String<detail::AlphabetAdaptor<AlphabetS3>> v;
    reserve(v, size(rng));
    for (auto c : rng) {
        auto t = detail::AlphabetAdaptor<AlphabetS3>{};
        t.value = c.to_rank();
        writeValue(v, t);
    }
    return v;
}

template <typename AlphabetS3 = seqan3::dna5>
struct writer {
    /** Wrapper to allow path and stream inputs
     */
    struct Output {
        std::string path;
        seqan::SeqFileOut fileOut{seqan::toCString(path)};

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


    // internal variables
    template <typename id_t, typename seq_t>
        requires std::ranges::range<id_t> and std::ranges::range<seq_t>
                 and requires(id_t id, seq_t seq) {
                    { *(id.begin()) } -> std::convertible_to<char>;
                    { *(seq.begin()) } -> std::convertible_to<AlphabetS3>;
                 }
    void emplace_back(id_t const& id, seq_t const& seq) {
        writeRecord(output.fileOut, id, toSeqan2(seq));
    }
};

}
}
