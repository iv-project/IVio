#pragma once

#include "alphabet_seqan223.h"

#include <filesystem>
#include <optional>
#include <seqan/bam_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/alphabet/cigar/cigar.hpp>
#include <seqan3/alphabet/quality/phred42.hpp>
#include <string_view>


namespace io2 {

namespace sam_io {


template <typename AlphabetS3, typename T>
auto toSeqan3(seqan::String<T> const& v) {
    return to_view(v) | std::views::transform([](auto const& v) {
        return seqan3::assign_char_to(static_cast<char>(v), AlphabetS3{});
    });
}

auto toSeqan3(seqan::String<seqan::CigarElement<>> const& v) {
    return to_view(v) | std::views::transform([](auto const& v) {
        using namespace seqan3::literals;
        seqan3::cigar::operation letter{};
        letter.assign_char(v.operation);
        return seqan3::cigar{v.count, letter};
    });
}



/* A single record
 *
 * This record represents a single entry in the file.
 * It provides views into the file.
 */
template <typename AlphabetS3, typename QualitiesS3>
struct record {
    // views for string types
    using sequence_view  = decltype(toSeqan3<AlphabetS3>(decltype(seqan::BamAlignmentRecord{}.seq){}));
    using cigar_view     = decltype(toSeqan3(decltype(seqan::BamAlignmentRecord{}.cigar){}));
    using qualities_view = decltype(toSeqan3<QualitiesS3>(decltype(seqan::BamAlignmentRecord{}.qual){}));


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
        std::string path;
        seqan::BamFileIn fileIn{seqan::toCString(path)};
        Input() {
        }

        Input(char const* _path)
            : Input(std::string{_path})
        {}
        Input(std::string const& _path)
            : path{_path}
        {
            seqan::BamHeader header;
            readHeader(header, fileIn);
        }
        Input(std::filesystem::path const& _path)
            : Input(_path.string())
        {}
    };

    /** Iterator that enables iterating over the reader
     */
    struct iter {
        reader& reader_;
        std::optional<record<AlphabetS3, QualitiesS3>> nextItem = reader_.next();

        auto operator*() const {
           return *nextItem;
        }
        auto operator++() -> iter& {
            nextItem = reader_.next();
            return *this;
        }
        auto operator!=(std::nullptr_t _end) const {
            return nextItem.has_value();
        }
    };
    // configurable from the outside
    Input input;
    [[no_unique_address]] detail::empty_class<AlphabetS3>  alphabet{};
    [[no_unique_address]] detail::empty_class<QualitiesS3> qualities{};


    // internal variables
    // storage for one record
    struct {
        seqan::BamAlignmentRecord record;
    } storage;

    auto next() -> std::optional<record<AlphabetS3, QualitiesS3>> {
        if (atEnd(input.fileIn)) return std::nullopt;
        readRecord(storage.record, input.fileIn);


        auto const& r = storage.record; // shorter name

        return record<AlphabetS3, QualitiesS3> {
            .qname    = to_view(r.qName),
            .flag     = static_cast<uint16_t>(r.flag),
            .rID      = r.rID,
            .beginPos = r.beginPos,
            .mapQ     = static_cast<uint8_t>(r.mapQ),
            .bin      = static_cast<uint16_t>(r.bin),
            .cigar    = toSeqan3(r.cigar),
            .rNextId  = r.rNextId,
            .pNext    = r.pNext,
            .tLen     = r.tLen,
            .seq      = toSeqan3<AlphabetS3>(storage.record.seq),
            .qual     = toSeqan3<QualitiesS3>(storage.record.qual),
            .tags     = to_view(r.tags),
        };
    }

    friend auto begin(reader& _reader) {
        return iter{.reader_ = _reader};
    }
    friend auto end(reader const&) {
        return nullptr;
    }

};

}
}
