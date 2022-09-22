#pragma once

#include "common.h"
#include "fasta_reader.h"
#include "fastq_reader.h"
#include "typed_range.h"

namespace io2::seq_io {

template <typename AlphabetS3, typename QualitiesS3>
struct record;


/**\brief A view onto a single record
 *
 * This record represents a single entry in the file.
 */
template <typename AlphabetS3, typename QualitiesS3>
struct record_view {
    using sequence_view  = typed_range<AlphabetS3>;
    using qualities_view = typed_range<QualitiesS3>;
    using record         = seq_io::record<AlphabetS3, QualitiesS3>;

    std::string_view id;
    sequence_view    seq;
    qualities_view   qual;
};

/**\brief A copy of a seq_io record
 */
template <typename AlphabetS3, typename QualitiesS3>
struct record {
    using sequence_t  = std::vector<AlphabetS3>;
    using qualities_t = std::vector<QualitiesS3>;
    using record_view = seq_io::record_view<AlphabetS3, QualitiesS3>;

    std::string id;
    sequence_t  seq;
    qualities_t qual;

    record(record_view v)
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

/** A reader to read sequence files like fasta, fastq, genbank, embl
 *
 * Usage:
 *    auto reader = io2::seq_io::reader {
 *       .input     = _file,                     // accepts string and streams
 *       .alphabet  = io2::type<seqan3::dna5>,   // default dna5
 *       .qualities = io2::type<seqan::phred42>, // default phred42
 *   };
 */
template <typename AlphabetS3 = seqan3::dna5,
          typename QualitiesS3 = seqan3::phred42>
struct reader {
    using record_view = seq_io::record_view<AlphabetS3, QualitiesS3>;
    using record      = seq_io::record<AlphabetS3, QualitiesS3>;

    // configurable from the outside
//    io2::Input<seqan::SeqFileIn> input;
    std::filesystem::path input;
    AlphabetS3            alphabet_type{};
    QualitiesS3           qualities_type{};


    fasta_io::reader<AlphabetS3>              fasta;
    fastq_io::reader<AlphabetS3, QualitiesS3> fastq;

    static auto extensions() -> std::vector<std::string> {
        static auto list = [&]() {
            auto l = std::vector<std::string>{};
            for (auto e : decltype(fasta)::extensions()) l.push_back(e);
            for (auto e : decltype(fastq)::extensions()) l.push_back(e);
            return l;
        }();
        return list;
    }

    static bool validExt(std::filesystem::path const& p) {
        return io2::validExtension(p, extensions());
    }


    // internal variables
    // storage for one record
    struct {
        record_view return_record;
    } storage;


    std::function<record_view const*()> next;
    void* ctor = [this]() {
/*        auto checkExtension = [this](auto& reader) {
            for (auto e : reader.extensions()) {
                auto s = input.filename().string();
                if (s.size() < e.size()) continue;
                s = s.substr(s.size() - e.size());
                if (s == e) {
                    return true;
                }
            }
            return false;
        };*/
        if (validExtension(input, decltype(fasta)::extensions())) {
            fasta.input = input;
            next = [this]() -> record_view const* {
                auto r = fasta.next();
                if (r == nullptr) return nullptr;
                storage.return_record = record_view {
                    .id  = r->id,
                    .seq = r->seq,
                };
                return &storage.return_record;
            };
        } else if (validExtension(input, decltype(fastq)::extensions())) {
            fastq.input = input;
            next = [this]() -> record_view const* {
                auto r = fastq.next();
                if (r == nullptr) return nullptr;
                storage.return_record = record_view {
                    .id   = r->id,
                    .seq  = r->seq,
                    .qual = r->qual,
                };
                return &storage.return_record;
            };
        } else {
            throw std::runtime_error("unknown file format");
        }
        return nullptr;
    }();

    using iterator = detail::iterator<reader, record_view, record>;
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

}
