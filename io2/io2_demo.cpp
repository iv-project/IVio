#include "io2/seq_io.h"
#include "io2/sam_io.h"

#include <seqan3/core/debug_stream.hpp>
#include <seqan3/alphabet/all.hpp>

int main(int argc, char** argv) {
    // call ./io2 read|write <file>

    if (argc != 3) return -1;

    auto action = std::string{argv[1]};
    auto file   = std::filesystem::path{argv[2]};

    // demonstrating reading a fasta file
    if (action == "read" and io2::seq_io::validExtension(file)) {
        auto reader = io2::seq_io::reader {
            .input     = file,
            .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
            .qualities = io2::type<seqan3::phred42>, // default is phred42
        };
        for (auto && record : reader) { // or use: for (auto& [id, seq] : reader) {
            seqan3::debug_stream << record.id << "\n";
            seqan3::debug_stream << record.seq << "\n";
            seqan3::debug_stream << record.qual << "\n";
        }
    }

    // demonstrating writing a fasta file
    if (action == "write" and io2::seq_io::validExtension(file)) {
        auto writer = io2::seq_io::writer {
            .output = file,
            .alphabet = io2::type<seqan3::dna5>,
        };

        std::string id="blub";
        using namespace seqan3::literals;
        std::vector<seqan3::dna5> seq = "ACCGGTT"_dna5;
        writer.emplace_back(id, seq);
    }



    // demonstrating reading a bam file
    if (action == "read" and (file.extension() == ".bam" or file.extension() == ".sam")) {
        auto reader = io2::sam_io::reader {
            .input     = file,
            .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
            .qualities = io2::type<seqan3::phred42>, // default is phred42
        };
        for (auto && record : reader) {
            seqan3::debug_stream << record.id << "\n";
            seqan3::debug_stream << record.seq << "\n";
            seqan3::debug_stream << record.cigar << "\n";
            seqan3::debug_stream << record.qual << "\n";
            seqan3::debug_stream << record.tags << "\n";
        }
    }

    // demonstrating writing a bam file
    if (action == "write" and (file.extension() == ".bam" or file.extension() == ".sam")) {
        auto writer = io2::sam_io::writer {
            .output     = file,
//            .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
//            .qualities = io2::type<seqan3::phred42>, // default is phred42
        };

        std::string id="blub";
        using namespace seqan3::literals;
        std::vector<seqan3::dna5> seq = "ACCGGTT"_dna5;
        writer.write({
            .id  = id,
            .seq = seq,
        });
        // alternative syntax:
//        writer.emplace_back(id, seq);

    }



}
