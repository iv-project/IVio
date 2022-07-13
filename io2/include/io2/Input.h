#pragma once

#include <filesystem>
#include <seqan/seq_io.h>
#include <seqan/bam_io.h>
#include <seqan/vcf_io.h>
#include <variant>

namespace io2 {

/** Wrapper to allow path and stream inputs
 */
template <typename File, typename ...Args>
struct Input {
    using Stream = seqan::Iter<std::istream, seqan::StreamIterator<seqan::Input>>;

    std::variant<File, Stream> fileIn;

    // function that avoids std::variant lookup
    using F = std::function<void(Args...)>;
    F readRecord;

    bool atEnd{false};


    Input(char const* _path)
        : fileIn{std::in_place_index<0>, _path}
    {
        readRecord = [&](auto&...args) {
            auto& file = std::get<0>(fileIn);
            seqan::readRecord(args..., file);
            atEnd = seqan::atEnd(file);
        };
    }
    Input(std::string const& _path)
        : Input(_path.c_str())
    {}
    Input(std::filesystem::path const& _path)
        : Input(_path.c_str())
    {}
    Input(std::istream& istr)
        : fileIn{std::in_place_index<1>, istr}
    {
        readRecord = [&](auto&...args) {
            auto& stream = std::get<1>(fileIn);
            seqan::readRecord(args..., stream, seqan::Fasta{});
            atEnd = seqan::atEnd(stream);
        };
    }
};

}
