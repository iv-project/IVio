#pragma once

#include "alphabet_seqan223.h"
#include "utils.h"

#include <filesystem>
#include <seqan/bam_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <string_view>


namespace io2 {

namespace sam_io {

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

    void emplace_back(range_over<char> auto const& id, range_over<AlphabetS3> auto const& seq) {
        seqan::BamAlignmentRecord record;
        record.qName = id;
        record.seq   = toSeqan2(seq);
        writeRecord(output.fileOut, record);
    }
};

}
}
