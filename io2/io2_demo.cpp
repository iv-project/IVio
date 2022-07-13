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

    // read data and print to the terminal
    for (auto && record : reader) {
        seqan3::debug_stream << record.id << "\n";
        seqan3::debug_stream << record.seq << "\n";
        seqan3::debug_stream << record.qual << "\n";
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

    // write a single entry.
    writer.write({
        .id   = id,
        .seq  = seq,
//        .qual = qual,
   });
}

void readAndWriteSeqIo(std::filesystem::path file, std::filesystem::path outFile) {
    // setup reader
    auto reader = io2::seq_io::reader {
        .input     = file,
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // setup writer
    auto writer = io2::seq_io::writer {
        .output = outFile,
        .alphabet = io2::type<seqan3::dna15>,
    };

    // reads entries and reduces the alphabet to 'A' and 'C'
    for (auto && r : reader) {
        writer.write({
            .id   = r.id,
            .seq  = r.seq | std::views::transform([](auto c) {
                // Only allows 'A' and 'C'
                return seqan3::assign_rank_to(c.to_rank() % 2, c);
            }),
            .qual = r.qual,
        });
    }
}

void readAndWriteStreamSeqIo() {
    // setup reader
    auto reader = io2::seq_io::reader {
        .input     = {std::cin, io2::seq_io::format::Fasta},
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // setup writer
    auto writer = io2::seq_io::writer {
        .output = {std::cout, io2::seq_io::format::Fasta},
        .alphabet = io2::type<seqan3::dna15>,
    };

    // reads entries and reduces the alphabet to 'A' and 'C'
    for (auto && r : reader) {
        writer.write({
            .id   = r.id,
            .seq  = r.seq | std::views::transform([](auto c) {
                // Only allows 'A' and 'C'
                return seqan3::assign_rank_to(c.to_rank() % 2, c);
            }),
            .qual = r.qual,
        });
    }
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


    // writting data to file
    writer.write({
        .id       = id,
//        .flag     = ?,
//        .rID      = ?,
//        .beginPos = ?,
//        .mapQ     = ?,
//        .bin      = ?,
//        .cigar    = ?,
//        .rNextId  = ?,
//        .pNext    = ?,
//        .tLen     = ?,
        .seq      = seq,
//        .qual     = ?,
//        .tags     = ?,
    });
}

void readAndWriteSamIo(std::filesystem::path file, std::filesystem::path outFile) {
    // setup reader
    auto reader = io2::sam_io::reader {
        .input     = file,
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // setup writer
    auto writer = io2::sam_io::writer {
        .output    = outFile,
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };


    // copy records
    for (auto && r : reader) {
        writer.write({
            .id       = r.id,
            .flag     = r.flag,
            .rID      = r.rID,
            .beginPos = r.beginPos,
            .mapQ     = r.mapQ,
            .bin      = r.bin,
            .cigar    = r.cigar,
            .rNextId  = r.rNextId,
            .pNext    = r.pNext,
            .tLen     = r.tLen,
            .seq      = r.seq,
            .qual     = r.qual | std::views::transform([](auto q) {
                // decrease quality by half
                return seqan3::assign_rank_to(q.to_rank() / 2, q);
            }),
            .tags     = r.tags,
        });
    }
}

void readAndWriteStreamSamIo() {
    // setup reader
    auto reader = io2::sam_io::reader {
        .input     = {std::cin, io2::sam_io::format::Sam},
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // setup writer
    auto writer = io2::sam_io::writer {
        .output    = {std::cout, io2::sam_io::format::Sam},
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };


    // copy records
    for (auto && r : reader) {
        writer.write({
            .id       = r.id,
            .flag     = r.flag,
            .rID      = r.rID,
            .beginPos = r.beginPos,
            .mapQ     = r.mapQ,
            .bin      = r.bin,
            .cigar    = r.cigar,
            .rNextId  = r.rNextId,
            .pNext    = r.pNext,
            .tLen     = r.tLen,
            .seq      = r.seq,
            .qual     = r.qual | std::views::transform([](auto q) {
                // decrease quality by half
                return seqan3::assign_rank_to(q.to_rank() / 2, q);
            }),
            .tags     = r.tags,
        });
    }
}

void readAndWriteStreamBamIo() {
    // setup reader
    auto reader = io2::sam_io::reader {
        .input     = {std::cin, io2::sam_io::format::Bam},
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };

    // setup writer
    auto writer = io2::sam_io::writer {
        .output    = {std::cout, io2::sam_io::format::Bam},
        .alphabet  = io2::type<seqan3::dna15>,   // default is dna5
        .qualities = io2::type<seqan3::phred42>, // default is phred42
    };


    // copy records
    for (auto && r : reader) {
        writer.write({
            .id       = r.id,
            .flag     = r.flag,
            .rID      = r.rID,
            .beginPos = r.beginPos,
            .mapQ     = r.mapQ,
            .bin      = r.bin,
            .cigar    = r.cigar,
            .rNextId  = r.rNextId,
            .pNext    = r.pNext,
            .tLen     = r.tLen,
            .seq      = r.seq,
            .qual     = r.qual | std::views::transform([](auto q) {
                // decrease quality by half
                return seqan3::assign_rank_to(q.to_rank() / 2, q);
            }),
            .tags     = r.tags,
        });
    }
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

    if (argc < 2) return -1;

    auto action = std::string{argv[1]};
    auto file   = std::filesystem::path{argv[2]};

    if (action == "read" and io2::seq_io::validExtension(file)) {
        readSeqIo(file);
    } else if (action == "write" and io2::seq_io::validExtension(file)) {
        writeSeqIo(file);
    } else if (action == "read_and_write" and io2::seq_io::validExtension(file) and argc == 4) {
        auto outFile = std::filesystem::path{argv[3]};
        readAndWriteSeqIo(file, outFile);
    } else if (action == "read_and_write" and file == "seq" and argc == 3) {
        readAndWriteStreamSeqIo();
    } else if (action == "read_and_copy" and io2::seq_io::validExtension(file)) {
        readAndCopySeqIo(file);
    } else if (action == "read_complete_file" and io2::seq_io::validExtension(file)) {
        readCompleteFileSeqIo(file);
    } else if (action == "read" and io2::sam_io::validExtension(file)) {
        readSamIo(file);
    } else if (action == "write" and io2::sam_io::validExtension(file)) {
        writeSamIo(file);
    } else if (action == "read_and_write" and io2::sam_io::validExtension(file) and argc == 4) {
        auto outFile = std::filesystem::path{argv[3]};
        readAndWriteSamIo(file, outFile);
    } else if (action == "read_and_write_stream" and file == "sam" and argc == 3) {
        readAndWriteStreamSamIo();
    } else if (action == "read_and_write_stream" and file == "bam" and argc == 3) {
        readAndWriteStreamBamIo();
    } else if (action == "read_and_copy" and io2::sam_io::validExtension(file)) {
        readAndCopySamIo(file);
    } else if (action == "read_complete_file" and io2::sam_io::validExtension(file)) {
        readCompleteFileSamIo(file);
    }

}
