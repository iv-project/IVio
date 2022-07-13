#pragma once

#include <filesystem>

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
};

}
