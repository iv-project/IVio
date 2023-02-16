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
    };

    writer(config config_);
    ~writer();

    void writeHeader(std::string_view key, std::string_view value);
    void addGenotype(std::string genotype);
    void write(record_view record);
};

}
