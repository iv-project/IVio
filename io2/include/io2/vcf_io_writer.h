#pragma once

#include "common.h"
#include "alphabet_seqan223.h"
#include "utils.h"
#include "Output.h"
#include "typed_range.h"

#include <seqan/bam_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <span>

namespace io2::vcf_io {

template <typename AlphabetS3 = seqan3::dna5>
struct writer {

    // configurable from the outside
    Output<seqan::VcfFileOut> output;
    [[no_unique_address]] detail::empty_class<AlphabetS3>  alphabet{};


    int x = [this]() {
        appendValue(contigNames(context(output.fileOut)), "ABC");
        appendValue(contigNames(context(output.fileOut)), "ABC");
        appendValue(contigNames(context(output.fileOut)), "ABC");
        appendValue(contigNames(context(output.fileOut)), "ABC");
        return 0;
    }();

    struct record {
        int32_t                             rID{};
        std::optional<int32_t>              beginPos;
        sized_typed_range<char>             id;
        sized_typed_range<AlphabetS3>       ref;
        sized_typed_range<AlphabetS3>       alt;
        std::optional<float>                qual;
        sized_typed_range<char>             filter;
        sized_typed_range<char>             info;
        sized_typed_range<char>             format;
        sized_typed_range<std::string_view> genotypeInfos;
    };

    void write(record _record) {
        seqan::VcfRecord r;
        r.rID           = _record.rID;
        r.beginPos      = _record.beginPos.value_or(seqan::VcfRecord::INVALID_POS);
        r.id            = detail::convert_to_seqan2_string(_record.id);
        r.ref           = detail::convert_to_seqan2_alphabet(_record.ref);
        r.alt           = detail::convert_to_seqan2_alphabet(_record.alt);
        r.qual          = _record.qual.value_or(seqan::VcfRecord::MISSING_QUAL());
        r.filter        = detail::convert_to_seqan2_string(_record.filter);
        r.info          = detail::convert_to_seqan2_string(_record.info);
        r.format        = detail::convert_to_seqan2_string(_record.format);
        for (auto info : _record.genotypeInfos) {
            appendValue(r.genotypeInfos, detail::convert_to_seqan2_string(info));
        }

        writeRecord(output.fileOut, r);
    }

    template <typename record_like>
    void write(record_like const& _record) {
        write(record {
            .rID           = _record.rID,
            .beginPos      = _record.beginPos,
            .id            = _record.id,
            .ref           = _record.ref,
            .alt           = _record.alt,
            .qual          = _record.qual,
            .filter        = _record.filter,
            .info          = _record.info,
            .format        = _record.format,
            .genotypeInfos = _record.genotypeInfos,
        });
    }

};

}
