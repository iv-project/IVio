// -----------------------------------------------------------------------------------------------------
// Copyright (c) 2006-2023, Knut Reinert & Freie Universität Berlin
// Copyright (c) 2016-2023, Knut Reinert & MPI für molekulare Genetik
// This file may be used, modified and/or redistributed under the terms of the 3-clause BSD-License
// shipped with this file.
// -----------------------------------------------------------------------------------------------------
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
