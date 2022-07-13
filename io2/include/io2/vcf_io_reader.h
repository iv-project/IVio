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


namespace io2::vcf_io {

enum class format {
    Vcf,
    Bcf,
};

/**
 * \noapi
 */
void convert_format(format _format, auto&& cb) {
    switch(_format) {
    case format::Vcf: cb(seqan::Vcf()); break;
    case format::Bcf: cb(seqan::Bcf()); break;
    }
}

/* A single record_view
 *
 * This record represents a single entry in the file.
 */
template <typename AlphabetS3>
struct record_view {
    // views for string types
    using sequence_view  = decltype(detail::convert_string_to_seqan3_view<AlphabetS3>({}));

    int32_t                       rID;
    int32_t                       beginPos;
    std::string_view              id;
    sequence_view                 ref;
    sequence_view                 alt;
    float                         qual;
    std::string_view              filter;
    std::string_view              info;
    std::string_view              format;
    std::vector<std::string_view> genotypeInfos;
};

/** A reader to read sequence files like fasta, fastq, genbank, embl
 *
 * Usage:
 *    auto reader = io2::seq_io::reader {
 *       .input = _file,                         // accepts string and streams
 *       .alphabet = sgg_io::type<seqan3::dna5>, // default dna5
 *   };
 */
template <typename AlphabetS3 = seqan3::dna5>
struct reader {
    struct record {
        // views for string types
        using sequence_t  = std::vector<AlphabetS3>;

        int32_t                  rID;
        int32_t                  beginPos;
        std::string              id;
        sequence_t               ref;
        sequence_t               alt;
        float                    qual;
        std::string              filter;
        std::string              info;
        std::string              format;
        std::vector<std::string> genotypeInfos;


        record(record_view<AlphabetS3> v)
            : rID{v.rID}
            , beginPos{v.beginPos}
            , id{v.id}
            , ref{v.ref | seqan3::ranges::to<std::vector>()}
            , alt{v.alt | seqan3::ranges::to<std::vector>()}
            , qual{v.qual}
            , filter{v.filter}
            , info{v.info}
            , format{v.format}
            //!TODO v.genotypeInfos
        {}
        record() = default;
        record(record const&) = default;
        record(record&&) = default;
        record& operator=(record const&) = default;
        record& operator=(record&&) = default;
    };

    // configurable from the outside
    io2::Input<seqan::VcfFileIn> input;
    [[no_unique_address]] detail::empty_class<AlphabetS3>  alphabet{};

    int x = [this]() {
        seqan::VcfHeader header;
        seqan::readHeader(header, input.fileIn);
        return 0;
    }();


    // internal variables
    // storage for one record
    struct {
        seqan::VcfRecord        seqan2_record;
        record_view<AlphabetS3> return_record;
    } storage;

    auto next() -> record_view<AlphabetS3> const* {
        if (input.atEnd()) return nullptr;
        input.readRecord(storage.seqan2_record);


        auto const& r = storage.seqan2_record; // shorter name
        storage.return_record = record_view<AlphabetS3> {
            .rID      = r.rID,
            .beginPos = r.beginPos,
            .id       = detail::convert_to_view(r.id),
            .ref      = detail::convert_string_to_seqan3_view<AlphabetS3>(r.ref),
            .alt      = detail::convert_string_to_seqan3_view<AlphabetS3>(r.alt),
            .qual     = r.qual,
            .filter   = detail::convert_to_view(r.filter),
            .info     = detail::convert_to_view(r.info),
            .format     = detail::convert_to_view(r.format),
            //!TODO genotypeInfos
        };
        return &storage.return_record;
    }

    using iterator = detail::iterator<reader, record_view<AlphabetS3>, record>;
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

template <typename AlphabetS3>
using record = reader<AlphabetS3>::record;

}
