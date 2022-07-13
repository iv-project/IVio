#pragma once

#include <filesystem>
#include <ostream>

namespace io2 {

/** Wrapper to allow path and stream inputs
 */
template <typename FileOut>
struct Output {
    FileOut fileOut;

    Output(char const* _path)
        : fileOut{_path}
    {}

    Output(std::string const& _path)
        : Output(_path.c_str())
    {}

    Output(std::filesystem::path const& _path)
        : Output(_path.c_str())
    {}

    template <typename format_t>
    Output(std::ostream& ostr, format_t format) {
        if (!open(fileOut, ostr, format)) {
            throw std::runtime_error("couldn't open ostream");
        }
    }

};

}
