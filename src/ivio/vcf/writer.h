#pragma once

#include "../writer_base.h"
#include "reader.h"

#include <filesystem>
#include <ostream>
#include <variant>

namespace ivio::vcf {

struct writer : writer_base<writer> {
    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::ostream>> output;

        // This is only relevant if a stream is being used
        bool compressed{};

        // Header
        vcf::header header{};
    };

    writer(config config_);
    ~writer();

    void write(record_view record);
};

}
