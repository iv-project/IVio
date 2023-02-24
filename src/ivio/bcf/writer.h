#pragma once

#include "../writer_base.h"
#include "header.h"
#include "record.h"

#include <filesystem>
#include <functional>
#include <ostream>
#include <memory>
#include <variant>
#include <span>

namespace ivio::bcf {

struct writer : writer_base<writer> {
    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::ostream>> output;

        // Header
        bcf::header header{};
    };

    writer(config config_);
    ~writer();

//    void writeHeader(std::string_view v);
    void write(record_view record);
};

}
