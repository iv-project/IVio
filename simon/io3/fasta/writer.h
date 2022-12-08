#pragma once

#include "record.h"

#include <filesystem>
#include <functional>
#include <ostream>
#include <memory>
#include <variant>


namespace io3::fasta {

struct writer_config {
    // Source: file or stream
    std::variant<std::filesystem::path, std::reference_wrapper<std::ostream>> output;

    // This is only relevant if a stream is being used
    bool compressed{};

    size_t length{80}; // Break after 80 characters
};


struct writer_pimpl;
struct writer {
private:
    std::unique_ptr<writer_pimpl> pimpl;

public:
    writer(writer_config config);
    ~writer();
    void write(record_view record);
};

}
