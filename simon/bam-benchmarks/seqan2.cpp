#include <seqan/bam_io.h>

using namespace seqan;

void seqan2_bench(std::string const& file) {
    BamFileIn fileIn(toCString(file));

    size_t a{};

    BamHeader header;
    readHeader(header, fileIn);

    BamAlignmentRecord record;
    while(!atEnd(fileIn)) {
        readRecord(record, fileIn);
        for (size_t i{0}; i < length(record.qName); ++i) {
            a += record.qName[i];
        }
        for (size_t i{0}; i < length(record.seq); ++i) {
            a += (size_t)record.seq[i];
        }
        for (size_t i{0}; i < length(record.cigar); ++i) {
            a += record.cigar[i].operation + record.cigar[i].count;
        }

    }
    std::cout << "total: " << a << "\n";
}
