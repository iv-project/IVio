#pragma once

#include "alphabet_seqan223.h"

#include <filesystem>
#include <optional>
#include <seqan/seq_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <string_view>


namespace io2 {

namespace seq_io {

//!TODO swap seq and id, this is just for demonstration/seqan3 compatbility, but not really needed

/* A single record
 *
 * This record represents a single entry in the file.
 * It provides views into the file.
 */
template <typename AlphabetS3>
struct record {
    std::string_view          id;
    sequence_view<AlphabetS3> seq;
    uint8_t                   qual; //!TODO
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
    /** Wrapper to allow path and stream inputs
     */
    struct Input {
        std::string path;
        seqan::SeqFileIn fileIn{seqan::toCString(path)};

        Input(char const* _path)
            : path{_path}
        {}
        Input(std::string const& _path)
            : path{_path}
        {}
        Input(std::filesystem::path const& _path)
            : path{_path.string()}
        {}
    };

    /** Iterator that enables iterating over the reader
     */
    struct iter {
        reader& reader_;
        std::optional<record<AlphabetS3>> nextItem = reader_.next();

        auto operator*() const -> record<AlphabetS3> {
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
    [[no_unique_address]] detail::empty_class<AlphabetS3> alphabet{};


    // internal variables
    // storage for one record
    struct {
        seqan::CharString id;
        seqan::String<detail::AlphabetAdaptor<AlphabetS3>> seq;
    } storage;

    auto next() -> std::optional<record<AlphabetS3>> {
        if (atEnd(input.fileIn)) return std::nullopt;
        readRecord(storage.id, storage.seq, input.fileIn);

        return record<AlphabetS3> {
            .id  = to_view(storage.id),
            .seq = toSeqan3(storage.seq),
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
