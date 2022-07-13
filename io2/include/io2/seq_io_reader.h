#pragma once

#include "common.h"
#include "alphabet_seqan223.h"
#include "iterator.h"
#include "Input.h"

#include <filesystem>
#include <optional>
#include <seqan/seq_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/alphabet/quality/phred42.hpp>
#include <seqan3/utility/range/to.hpp>
#include <string_view>
#include <variant>


namespace io2::seq_io {

enum class format {
    Fasta,
    Fastq,
    Genbank,
    Embl,
};

/**
 * \noapi
 */
void convert_format(format _format, auto&& cb) {
    switch(_format) {
    case format::Fasta:   cb(seqan::Fasta()); break;
    case format::Fastq:   cb(seqan::Fastq()); break;
    case format::Genbank: cb(seqan::GenBank()); break;
    case format::Embl:    cb(seqan::Embl()); break;
    }
}

/* A single view onto a record
 *
 * This record represents a single entry in the file.
 */
template <typename AlphabetS3, typename QualitiesS3>
struct record_view {
    using sequence_view  = decltype(detail::convert_to_seqan3_view<AlphabetS3>({}));
    using qualities_view = decltype(detail::convert_to_seqan3_view<QualitiesS3>({}));

    std::string_view id;
    sequence_view    seq;
    qualities_view   qual;
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
        using sequence_t  = std::vector<AlphabetS3>;
        using qualities_t = std::vector<QualitiesS3>;

        std::string id;
        sequence_t  seq;
        qualities_t qual;

        record(record_view<AlphabetS3, QualitiesS3> v)
            : id{v.id}
            , seq{v.seq | seqan3::ranges::to<std::vector>()}
            , qual{v.qual | seqan3::ranges::to<std::vector>()}
        {}
        record() = default;
        record(record const&) = default;
        record(record&&) = default;
        record& operator=(record const&) = default;
        record& operator=(record&&) = default;
    };

    // configurable from the outside
    io2::Input<seqan::SeqFileIn> input;
    [[no_unique_address]] detail::empty_class<AlphabetS3>  alphabet{};
    [[no_unique_address]] detail::empty_class<QualitiesS3> qualities{};


    // internal variables
    // storage for one record
    struct {
        seqan::CharString id;
        seqan::String<detail::AlphabetAdaptor<AlphabetS3>> seq;
        seqan::String<detail::AlphabetAdaptor<QualitiesS3>> qual;

        record_view<AlphabetS3, QualitiesS3> return_record;
    } storage;

    auto next() -> record_view<AlphabetS3, QualitiesS3> const* {
        if (input.atEnd()) return nullptr;
        input.readRecord(storage.id, storage.seq, storage.qual);

        storage.return_record = record_view<AlphabetS3, QualitiesS3> {
            .id   = detail::convert_to_view(storage.id),
            .seq  = detail::convert_to_seqan3_view(storage.seq),
            .qual = detail::convert_to_seqan3_view(storage.qual),
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
