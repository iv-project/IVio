#pragma once

#include <filesystem>
#include <functional>
#include <ostream>
#include <memory>
#include <variant>
#include <span>

#include "bcf_reader.h"


namespace io3::bcf {

using record_view = io3::vcf_reader_view_record_view;

struct writer_config {
    // Source: file or stream
    std::variant<std::filesystem::path, std::reference_wrapper<std::ostream>> output;

    // This is only relevant if a stream is being used
    bool compressed{};
};


struct writer_pimpl;
struct writer {
private:
    std::unique_ptr<writer_pimpl> pimpl;

public:
    writer(writer_config config);
    ~writer();
    void writeHeader(std::string_view v);
    void write(record_view record);
};

}
