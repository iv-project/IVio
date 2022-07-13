#pragma once

#include "common.h"
#include "alphabet_seqan223.h"
#include "utils.h"

#include <filesystem>
#include <seqan/seq_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <string_view>


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
    [[no_unique_address]] detail::empty_class<AlphabetS3> alphabet{};

    void emplace_back(range_over<char> auto const& id, range_over<AlphabetS3> auto const& seq) {
        writeRecord(output.fileOut, id, detail::convert_to_seqan2_alphabet(seq));
    }
};

}
