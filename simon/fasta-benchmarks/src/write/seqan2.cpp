#include <cassert>
#include <fstream>
#include <filesystem>
#include <seqan/seq_io.h>
#include <vector>

using namespace seqan;

void seqan2_bench(std::filesystem::path file, std::vector<std::tuple<std::string, std::vector<uint8_t>>> const& data) {

    SeqFileOut seqFileOut(file.c_str());

    CharString id;
    Dna5String seq;
    seqFileOut.context.options.lineLength = 80;
    for (auto const& [seq_id, seq_data] : data) {
        id = seq_id;
        resize(seq, seq_data.size());
        std::memcpy(toCString(seq), seq_data.data(), seq_data.size());
        writeRecord(seqFileOut, id, seq);
    }
}
