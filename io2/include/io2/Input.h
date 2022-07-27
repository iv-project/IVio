#pragma once

#include <filesystem>
#include <seqan/bam_io.h>
#include <seqan/seq_io.h>
#include <seqan/vcf_io.h>

namespace io2 {

/** Wrapper to allow path and stream inputs
 */
template <typename File>
struct Input {
    File fileIn;

    Input(char const* _path)
        : fileIn{_path}
    {}

    Input(std::string const& _path)
        : Input(_path.c_str())
    {}

    Input(std::filesystem::path const& _path)
        : Input(_path.c_str())
    {}

    template <typename format_t>
    Input(std::istream& istr, format_t format) {
        convert_format(format, [&](auto format) {
            assign(fileIn.format, format);
        });
        if (!open(fileIn, istr)) {
            throw std::runtime_error("couldn't open istream");
        }
    }

    bool atEnd() {
        return seqan::atEnd(fileIn);
    }

    template <typename... Args>
    void readRecord(Args&... args) {
        seqan::readRecord(args..., fileIn);
    }
};

}
