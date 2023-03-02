#include <filesystem>
#include <seqan/vcf_io.h>

using namespace seqan;

void seqan2_bench(std::filesystem::path pathIn, std::filesystem::path pathOut, size_t threadNbr) {
    //!TODO don't know how to set threadNbr properly
    VcfFileIn  fileIn(pathIn.c_str());
    VcfFileOut fileOut(pathOut.c_str());

    VcfHeader header;
    readHeader(header, fileIn);
    writeHeader(fileOut, header);

    VcfRecord record;
    while (!atEnd(fileIn)) {
        readRecord(record, fileIn);
        writeRecord(fileOut, record);
    }
}
