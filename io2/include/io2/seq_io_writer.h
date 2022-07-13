#pragma once

#include "common.h"
#include "alphabet_seqan223.h"
#include "utils.h"
#include "Output.h"

#include <seqan/seq_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/alphabet/quality/phred42.hpp>

namespace io2::seq_io {

template <typename AlphabetS3 = seqan3::dna5,
          typename QualitiesS3 = seqan3::phred42>
struct writer {
    // configurable from the outside
    Output<seqan::SeqFileOut> output;
    [[no_unique_address]] detail::empty_class<AlphabetS3> alphabet{};

    struct record {
        std::span<char>          id;
        std::span<AlphabetS3>    seq;
        std::span<QualitiesS3>   qual;
    };

    void write(record _record) {
        auto id   = detail::convert_to_seqan2_string(_record.id);
        auto seq  = detail::convert_to_seqan2_alphabet(_record.seq);
        auto qual = detail::convert_to_seqan2_qualities(_record.qual);

        writeRecord(output.fileOut, id, seq, qual);
    }
};

}
