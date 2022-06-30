#include "io2/seq_io_reader.h"
#include "io2/seq_io_writer.h"
#include "io2/sam_io_reader.h"
#include <seqan3/core/debug_stream.hpp>
#include <seqan3/alphabet/all.hpp>

int main(int argc, char** argv) {
    //FASTA output
/*    auto writer = io2::seq_io::writer {
        .output = "output.fastq",
        .alphabet = io2::type<seqan3::dna5>,
    };

    std::string id="blub";
    using namespace seqan3::literals;
    std::vector<seqan3::dna5> seq = "ACCGGTT"_dna5;
    writer.emplace_back(id, seq);*/


    //SAM input
    auto reader = io2::sam_io::reader {
        .input     = "sampled.bam",
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };
    for (auto && record : reader) {
        seqan3::debug_stream << record.qname << "\n";
        seqan3::debug_stream << record.seq << "\n";
        seqan3::debug_stream << record.cigar << "\n";
        seqan3::debug_stream << record.qual << "\n";
        seqan3::debug_stream << record.tags << "\n";
    }


}
