#pragma once

#include "common.h"
#include "alphabet_seqan223.h"
#include "iterator.h"

#include <filesystem>
#include <optional>
#include <seqan/bam_io.h>
#include <seqan3/alphabet/cigar/cigar.hpp>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/alphabet/quality/phred42.hpp>
#include <string_view>


namespace io2::sam_io {

auto toSeqan3(seqan::String<seqan::CigarElement<>> const& v) {
    return to_view(v) | std::views::transform([](auto const& v) {
        using namespace seqan3::literals;
        seqan3::cigar::operation letter{};
        letter.assign_char(v.operation);
        return seqan3::cigar{v.count, letter};
    });
}

/* A single record_view
 *
 * This record represents a single entry in the file.
 */
template <typename AlphabetS3, typename QualitiesS3>
struct record_view {
    // views for string types
    using sequence_view  = decltype(io2::toSeqan3<AlphabetS3>(decltype(seqan::BamAlignmentRecord{}.seq){}));
    using cigar_view     = decltype(toSeqan3(decltype(seqan::BamAlignmentRecord{}.cigar){}));
    using qualities_view = decltype(io2::toSeqan3<QualitiesS3>(decltype(seqan::BamAlignmentRecord{}.qual){}));

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
        record_view<AlphabetS3, QualitiesS3> return_record;
    } storage;

    auto next() -> record_view<AlphabetS3, QualitiesS3> const* {
        if (atEnd(input.fileIn)) return nullptr;
        readRecord(storage.seqan2_record, input.fileIn);


        auto const& r = storage.seqan2_record; // shorter name
        storage.return_record = record_view<AlphabetS3, QualitiesS3> {
            .id       = to_view(r.qName),
            .flag     = static_cast<uint16_t>(r.flag),
            .rID      = r.rID,
            .beginPos = r.beginPos,
            .mapQ     = static_cast<uint8_t>(r.mapQ),
            .bin      = static_cast<uint16_t>(r.bin),
            .cigar    = toSeqan3(r.cigar),
            .rNextId  = r.rNextId,
            .pNext    = r.pNext,
            .tLen     = r.tLen,
            .seq      = io2::toSeqan3<AlphabetS3>(r.seq),
            .qual     = io2::toSeqan3<QualitiesS3>(r.qual),
            .tags     = to_view(r.tags),
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
