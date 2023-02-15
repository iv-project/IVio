#include <cassert>
#include <fstream>
#include <seqan3/io/sequence_file/all.hpp>
#include <vector>

void seqan3_bench(std::filesystem::path file, std::vector<std::tuple<std::string, std::vector<uint8_t>>> const& data) {
    auto writer = seqan3::sequence_file_output{file};

    using types = seqan3::type_list<std::vector<seqan3::dna5>, std::string>;
    using fields = seqan3::fields<seqan3::field::seq, seqan3::field::id>;
    using sequence_record_type = seqan3::sequence_record<types, fields>;

    for (auto const& [seq_id, seq] : data) {
        auto seqan_data = reinterpret_cast<std::vector<seqan3::dna5> const&>(seq);
        auto record = sequence_record_type{seqan_data, seq_id};
        writer.push_back(record);
    }
}
