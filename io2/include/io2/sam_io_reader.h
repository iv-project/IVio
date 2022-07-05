#pragma once

#include "alphabet_seqan223.h"
#include "iterator.h"

#include <filesystem>
#include <optional>
#include <seqan/bam_io.h>
#include <seqan3/alphabet/cigar/cigar.hpp>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/alphabet/quality/phred42.hpp>
#include <string_view>


namespace io2::sam_io::detail {

auto toSeqan3(seqan::String<seqan::CigarElement<>> const& v) {
    return to_view(v) | std::views::transform([](auto const& v) {
        using namespace seqan3::literals;
        seqan3::cigar::operation letter{};
        letter.assign_char(v.operation);
        return seqan3::cigar{v.count, letter};
    });
}

}

namespace io2::sam_io {

/* A single record
 *
 * This record represents a single entry in the file.
 * It provides views into the file.
 */
template <typename AlphabetS3, typename QualitiesS3>
struct record {
    // views for string types
    using sequence_view  = decltype(io2::toSeqan3<AlphabetS3>(decltype(seqan::BamAlignmentRecord{}.seq){}));
    using cigar_view     = decltype(io2::sam_io::detail::toSeqan3(decltype(seqan::BamAlignmentRecord{}.cigar){}));
    using qualities_view = decltype(io2::toSeqan3<QualitiesS3>(decltype(seqan::BamAlignmentRecord{}.qual){}));

    std::string_view qname;
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
    /** Wrapper to allow path and stream inputs
     */
    struct Input {
        seqan::BamFileIn fileIn;

        Input(char const* _path)
            : fileIn{_path}
        {
            seqan::BamHeader header;
            readHeader(header, fileIn);
        }
        Input(std::string const& _path)
            : Input(_path.c_str())
        {}
        Input(std::filesystem::path const& _path)
            : Input(_path.c_str())
        {}
    };

    // configurable from the outside
    Input input;
    [[no_unique_address]] io2::detail::empty_class<AlphabetS3>  alphabet{};
    [[no_unique_address]] io2::detail::empty_class<QualitiesS3> qualities{};


    // internal variables
    // storage for one record
    struct {
        seqan::BamAlignmentRecord       seqan2_record;
        record<AlphabetS3, QualitiesS3> return_record;
    } storage;

    auto next() -> record<AlphabetS3, QualitiesS3> const* {
        if (atEnd(input.fileIn)) return nullptr;
        readRecord(storage.seqan2_record, input.fileIn);


        auto const& r = storage.seqan2_record; // shorter name
        storage.return_record = record<AlphabetS3, QualitiesS3> {
            .qname    = to_view(r.qName),
            .flag     = static_cast<uint16_t>(r.flag),
            .rID      = r.rID,
            .beginPos = r.beginPos,
            .mapQ     = static_cast<uint8_t>(r.mapQ),
            .bin      = static_cast<uint16_t>(r.bin),
            .cigar    = detail::toSeqan3(r.cigar),
            .rNextId  = r.rNextId,
            .pNext    = r.pNext,
            .tLen     = r.tLen,
            .seq      = toSeqan3<AlphabetS3>(r.seq),
            .qual     = toSeqan3<QualitiesS3>(r.qual),
            .tags     = to_view(r.tags),
        };
        return &storage.return_record;
    }

    friend auto begin(reader& _reader) {
        using iter = io2::detail::iterator<reader, record<AlphabetS3, QualitiesS3>>;
        return iter{.reader = _reader};
    }
    friend auto end(reader const&) {
        return nullptr;
    }

};

}
