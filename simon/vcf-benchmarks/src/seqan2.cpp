#include <seqan/vcf_io.h>

#include <seqan3/alphabet/nucleotide/dna5.hpp>

using namespace seqan;

void seqan2_bench(std::string_view _file) {
    auto file = std::string{_file};
    VcfFileIn fileIn(toCString(file));

    VcfHeader header;
    readHeader(header, fileIn);

    size_t ct{}, sum{};
    VcfRecord record;
    while(!atEnd(fileIn)) {
        readRecord(record, fileIn);
        ct += record.beginPos+1;
        for (auto c : record.ref) {
            auto v = seqan3::assign_char_to(ordValue(c), seqan3::dna5{});
            sum += v.to_rank();
        }
    }
    std::cout << "total: " << ct << " " << sum << "\n";
}

//void seqan2_vcf_bench(std::string const& file) {
//    VcfFileIn fileIn(toCString(file));
//
//    VcfHeader header;
//    readHeader(header, fileIn);
//
//    while(!atEnd(fileIn)) {
//        VcfRecord r;
//        readRecord(r, fileIn);
//        std::cout << r.rID << "\t";
//        std::cout << r.beginPos << "\t";
//        std::cout << r.id << "\t";
//        std::cout << r.ref << "\t";
//        std::cout << r.alt << "\t";
//        std::cout << r.qual << "\t";
//        std::cout << r.filter << "\t";
//        std::cout << r.info << "\t";
//        std::cout << r.format;
//        for (auto const& g : r.genotypeInfos) {
//            std::cout << "\t" << g;
//        }
//        std::cout << "\n";
//    }
//}
