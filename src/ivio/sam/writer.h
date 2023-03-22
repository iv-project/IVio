#pragma once

#include "../writer_base.h"
#include "record.h"

#include <filesystem>
#include <ostream>
#include <variant>
#include <vector>

namespace ivio::sam {

struct writer : writer_base<writer> {
    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::ostream>> output;

        // This is only relevant if a stream is being used
        bool compressed{};

        // Header
       std::vector<std::string> header{};
    };

    writer(config config_);
    ~writer();

    void write(record_view record);
    void close();
};

}
