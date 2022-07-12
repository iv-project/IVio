#include "io2/seq_io.h"
#include "io2/sam_io.h"

#include <seqan3/core/debug_stream.hpp>
#include <seqan3/alphabet/all.hpp>
#include <iterator>


void readSeqIo(std::filesystem::path file) {
    // setup reader
    auto reader = io2::seq_io::reader {
        .input     = file,
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // version 1:
    for (auto && record : reader) {
        seqan3::debug_stream << record.id << "\n";
        seqan3::debug_stream << record.seq << "\n";
        seqan3::debug_stream << record.qual << "\n";
    }

    // version 2: // maybe this should not be supported, since sam and vcf won't support it
    for (auto & [id, seq, qual] : reader) {
        seqan3::debug_stream << id << "\n";
        seqan3::debug_stream << seq << "\n";
        seqan3::debug_stream << qual << "\n";
    }
}

void writeSeqIo(std::filesystem::path file) {
    // setup writer
    auto writer = io2::seq_io::writer {
        .output = file,
        .alphabet = io2::type<seqan3::dna5>,
    };

    // generating some data
    std::string id="blub";
    using namespace seqan3::literals;
    std::vector<seqan3::dna5> seq = "ACCGGTT"_dna5;

    // write a single entry. (Maybe only a version as for writeSamIo should exists)
    writer.emplace_back(id, seq);
}

void readAndCopySeqIo(std::filesystem::path file) {
    // setup reader
    auto reader = io2::seq_io::reader {
        .input     = file,
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // list with all results
    auto results = std::vector<decltype(reader)::record>{};

    for (auto && record : reader) {
        results.emplace_back(record);
    }
}

void readCompleteFileSeqIo(std::filesystem::path file) {
    // single line to read complete file
    auto results = io2::seq_io::reader {
        .input     = file,
        .alphabet  = io2::type<seqan3::dna15>,
        .qualities = io2::type<seqan3::phred42>,
    } | seqan3::ranges::to<std::vector>();

    // or short version
    // auto results = io2::seq_io::reader<seqan3::dna15, seqan3::phred42>{file}
    //   | seqan3::ranges::to<std::vector>();

    for (auto const& r : results) {
        seqan3::debug_stream << r.id << "\n";
        seqan3::debug_stream << r.seq << "\n";
        seqan3::debug_stream << r.qual << "\n";
    }
}


void readSamIo(std::filesystem::path file) {
    // setup reader
    auto reader = io2::sam_io::reader {
        .input     = file,
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // read records
    for (auto && record : reader) {
        seqan3::debug_stream << record.id << "\n";
        seqan3::debug_stream << record.seq << "\n";
        seqan3::debug_stream << record.cigar << "\n";
        seqan3::debug_stream << record.qual << "\n";
        seqan3::debug_stream << record.tags << "\n";
    }
}

void writeSamIo(std::filesystem::path file) {
    // setup writer
    auto writer = io2::sam_io::writer {
        .output     = file,
//        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
//        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // generating some data
    std::string id="blub";
    using namespace seqan3::literals;
    std::vector<seqan3::dna5> seq = "ACCGGTT"_dna5;


    // version 1: writting
    writer.write({
        .id  = id,
        .seq = seq,
    });

    // version 2: writting (this will have very bad overview, with a lot of parameters)
    writer.emplace_back(id, seq);
}

void readAndCopySamIo(std::filesystem::path file) {
    // setup reader
    auto reader = io2::sam_io::reader {
        .input     = file,
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // list with all results
    auto results = std::vector<decltype(reader)::record>{};

    for (auto && record : reader) {
        results.emplace_back(record);
    }
}

void readCompleteFileSamIo(std::filesystem::path file) {
    // single line to read complete file
    auto results = io2::sam_io::reader {
        .input     = file,
        .alphabet  = io2::type<seqan3::dna15>,
        .qualities = io2::type<seqan3::phred42>,
    } | seqan3::ranges::to<std::vector>();

    // or short version
    // auto results = io2::sam_io::reader<seqan3::dna15, seqan3::phred42>{file}
    //   | seqan3::ranges::to<std::vector>();

    for (auto const& r : results) {
        seqan3::debug_stream << r.id << "\n";
        seqan3::debug_stream << r.seq << "\n";
        seqan3::debug_stream << r.cigar << "\n";
        seqan3::debug_stream << r.qual << "\n";
        seqan3::debug_stream << r.tags << "\n";
    }
}


int main(int argc, char** argv) {
    // call ./io2 read|write <file>

    if (argc != 3) return -1;

    auto action = std::string{argv[1]};
    auto file   = std::filesystem::path{argv[2]};

    if (action == "read" and io2::seq_io::validExtension(file)) {
        readSeqIo(file);
    } else if (action == "write" and io2::seq_io::validExtension(file)) {
        writeSeqIo(file);
    } else if (action == "read_and_copy" and io2::seq_io::validExtension(file)) {
        readAndCopySeqIo(file);
    } else if (action == "read_complete_file" and io2::seq_io::validExtension(file)) {
        readCompleteFileSeqIo(file);
    } else if (action == "read" and io2::sam_io::validExtension(file)) {
        readSamIo(file);
    } else if (action == "write" and io2::sam_io::validExtension(file)) {
        writeSamIo(file);
    } else if (action == "read_and_copy" and io2::sam_io::validExtension(file)) {
        readAndCopySamIo(file);
    } else if (action == "read_complete_file" and io2::sam_io::validExtension(file)) {
        readCompleteFileSamIo(file);
    }

}
