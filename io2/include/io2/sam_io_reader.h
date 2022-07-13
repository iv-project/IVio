#pragma once

#include "common.h"
#include "alphabet_seqan223.h"
#include "iterator.h"

#include <filesystem>
#include <optional>
#include <seqan/bam_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/alphabet/quality/phred42.hpp>
#include <string_view>


namespace io2::sam_io {

/* A single record_view
 *
 * This record represents a single entry in the file.
 */
template <typename AlphabetS3, typename QualitiesS3>
struct record_view {
    // views for string types
    using sequence_view  = decltype(detail::convert_to_seqan3_view<AlphabetS3>(decltype(seqan::BamAlignmentRecord{}.seq){}));
    using cigar_view     = decltype(detail::convert_to_seqan3_view(decltype(seqan::BamAlignmentRecord{}.cigar){}));
    using qualities_view = decltype(detail::convert_to_seqan3_view<QualitiesS3>(decltype(seqan::BamAlignmentRecord{}.qual){}));

    std::string_view id;
    uint16_t         flag;
    int32_t          rID;
    int32_t          beginPos;
    uint8_t          mapQ;
    uint16_t         bin;
    cigar_view       cigar;
    int32_t          rNextId;
    int32_t          pNext;
    int32_t          tLen;
    sequence_view    seq;
    qualities_view   qual;
    std::string_view tags;
};

/** A reader to read sequence files like fasta, fastq, genbank, embl
 *
 * Usage:
 *    auto reader = io2::seq_io::reader {
 *       .input = _file,                         // accepts string and streams
 *       .alphabet = sgg_io::type<seqan3::dna5>, // default dna5
 *   };
 */
template <typename AlphabetS3 = seqan3::dna5,
          typename QualitiesS3 = seqan3::phred42>
struct reader {
    struct record {
        // views for string types
        using sequence_t  = std::vector<AlphabetS3>;
        using cigar_t     = std::vector<seqan3::cigar>;
        using qualities_t = std::vector<QualitiesS3>;

        std::string   id;
        uint16_t      flag;
        int32_t       rID;
        int32_t       beginPos;
        uint8_t       mapQ;
        uint16_t      bin;
        cigar_t       cigar;
        int32_t       rNextId;
        int32_t       pNext;
        int32_t       tLen;
        sequence_t    seq;
        qualities_t   qual;
        std::string   tags;


        record(record_view<AlphabetS3, QualitiesS3> v)
            : id{v.id}
            , flag{v.flag}
            , rID{v.rID}
            , beginPos{v.beginPos}
            , mapQ{v.mapQ}
            , bin{v.bin}
            , cigar{v.cigar | seqan3::ranges::to<std::vector>()}
            , rNextId{v.rNextId}
            , pNext{v.pNext}
            , tLen{v.tLen}
            , seq{v.seq | seqan3::ranges::to<std::vector>()}
            , qual{v.qual | seqan3::ranges::to<std::vector>()}
            , tags{v.tags}
        {}
        record() = default;
        record(record const&) = default;
        record(record&&) = default;
        record& operator=(record const&) = default;
        record& operator=(record&&) = default;
    };

    using Input = io2::Input<seqan::BamFileIn, seqan::BamAlignmentRecord&>;

    // configurable from the outside
    std::type_identity<Input>::type input;
    [[no_unique_address]] detail::empty_class<AlphabetS3>  alphabet{};
    [[no_unique_address]] detail::empty_class<QualitiesS3> qualities{};


    // internal variables
    // storage for one record
    struct {
        seqan::BamAlignmentRecord            seqan2_record;
        record_view<AlphabetS3, QualitiesS3> return_record;
    } storage;

    auto next() -> record_view<AlphabetS3, QualitiesS3> const* {
        if (input.atEnd) return nullptr;
        input.readRecord(storage.seqan2_record);


        auto const& r = storage.seqan2_record; // shorter name
        storage.return_record = record_view<AlphabetS3, QualitiesS3> {
            .id       = detail::convert_to_view(r.qName),
            .flag     = static_cast<uint16_t>(r.flag),
            .rID      = r.rID,
            .beginPos = r.beginPos,
            .mapQ     = static_cast<uint8_t>(r.mapQ),
            .bin      = static_cast<uint16_t>(r.bin),
            .cigar    = detail::convert_to_seqan3_view(r.cigar),
            .rNextId  = r.rNextId,
            .pNext    = r.pNext,
            .tLen     = r.tLen,
            .seq      = detail::convert_to_seqan3_view<AlphabetS3>(r.seq),
            .qual     = detail::convert_to_seqan3_view<QualitiesS3>(r.qual),
            .tags     = detail::convert_to_view(r.tags),
        };
        return &storage.return_record;
    }

    using iterator = detail::iterator<reader, record_view<AlphabetS3, QualitiesS3>, record>;
    auto end() const {
        return iterator{.reader = nullptr};
    }

    friend auto begin(reader& _reader) {
        return iterator{.reader = &_reader};
    }
    friend auto end(reader const& _reader) {
        return _reader.end();
    }
};

template <typename AlphabetS3, typename QualitiesS3>
using record = reader<AlphabetS3, QualitiesS3>::record;

}
