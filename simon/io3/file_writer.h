#pragma once

#include "concepts.h"

#include <filesystem>
#include <fstream>

namespace io3 {

class file_writer {
protected:
    std::ofstream ofs;

public:
    file_writer(std::filesystem::path path)
        : ofs(path, std::ios_base::out | std::ios_base::binary)
    {}

    file_writer() = delete;
    file_writer(file_writer const&) = delete;
    file_writer(file_writer&& _other) noexcept = default;
    ~file_writer() = default;

    auto operator=(file_writer const&) -> file_writer& = delete;
    auto operator=(file_writer&&) -> file_writer& = delete;

    void write(std::span<char> buffer) {
        ofs.write(buffer.data(), buffer.size());
    }
};

static_assert(writer_c<file_writer>);
}
