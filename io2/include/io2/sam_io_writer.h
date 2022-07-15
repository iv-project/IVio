#pragma once

#include "common.h"
#include "alphabet_seqan223.h"
#include "utils.h"
#include "Output.h"
#include "typed_range.h"

#include <seqan/bam_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <span>

namespace io2::sam_io {

template <typename AlphabetS3 = seqan3::dna5,
          typename QualitiesS3 = seqan3::phred42>
struct writer {

    // configurable from the outside
    Output<seqan::BamFileOut> output;
    [[no_unique_address]] detail::empty_class<AlphabetS3>  alphabet{};
    [[no_unique_address]] detail::empty_class<QualitiesS3> qualities{};

    struct record {
        sized_typed_range<char>          id;
        uint16_t                         flag{};
        std::optional<int32_t>           rID;
        std::optional<int32_t>           beginPos;
        uint8_t                          mapQ{};
        uint16_t                         bin{};
        sized_typed_range<seqan3::cigar> cigar;
        std::optional<int32_t>           rNextId{};
        int32_t                          pNext{};
        std::optional<int32_t>           tLen{};
        sized_typed_range<AlphabetS3>    seq;
        sized_typed_range<QualitiesS3>   qual;
        sized_typed_range<char>          tags;
    };

    void write(record _record) {
        seqan::BamAlignmentRecord r;
        r.qName    = detail::convert_to_seqan2_string(_record.id);
        r.flag     = _record.flag;
        r.rID      = _record.rID.value_or(seqan::BamAlignmentRecord::INVALID_REFID);
        r.beginPos = _record.beginPos.value_or(seqan::BamAlignmentRecord::INVALID_POS);
        r.mapQ     = _record.mapQ;
        r.bin      = _record.bin;
        r.cigar    = detail::convert_to_seqan2_cigar(_record.cigar);
        r.rNextId  = _record.rNextId.value_or(seqan::BamAlignmentRecord::INVALID_REFID);
        r.pNext    = _record.pNext;
        r.tLen     = _record.tLen.value_or(seqan::BamAlignmentRecord::INVALID_LEN);
        r.seq      = detail::convert_to_seqan2_alphabet(_record.seq);
        r.qual     = detail::convert_to_seqan2_qualities(_record.qual);
        r.tags     = detail::convert_to_seqan2_string(_record.tags);

        writeRecord(output.fileOut, r);
    }

    template <typename record_like>
    void write(record_like const& _record) {
        write(record {
            .id       = _record.id,
            .flag     = _record.flag,
            .rID      = _record.rID,
            .beginPos = _record.beginPos,
            .mapQ     = _record.mapQ,
            .bin      = _record.bin,
            .cigar    = _record.cigar,
            .rNextId  = _record.rNextId,
            .pNext    = _record.pNext,
            .tLen     = _record.tLen,
            .seq      = _record.seq,
            .qual     = _record.qual,
            .tags     = _record.tags,
        });
    }

};

}
