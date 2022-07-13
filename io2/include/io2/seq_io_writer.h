#pragma once

#include "common.h"
#include "alphabet_seqan223.h"
#include "utils.h"

#include <filesystem>
#include <seqan/seq_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <string_view>


namespace io2::seq_io::detail {

inline auto toSeqan2(std::ranges::range auto rng) {
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

}

namespace io2::seq_io {

template <typename AlphabetS3 = seqan3::dna5>
struct writer {
    /** Wrapper to allow path and stream inputs
     */
    struct Output {
        seqan::SeqFileOut fileOut;

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

    void emplace_back(range_over<char> auto const& id, range_over<AlphabetS3> auto const& seq) {
        writeRecord(output.fileOut, id, detail::toSeqan2(seq));
    }
};

}
