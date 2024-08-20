// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "../detail/concepts.h"
#include "../detail/writer_base.h"
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
    using record_view = bcf::record_view; //!doc: see record_writer_c<writer> concept

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::ostream>> output;

        // Header
        bcf::header header{};
    };

    writer(config config_);
    ~writer();

//    void writeHeader(std::string_view v);

    //!doc: see record_writer_c<writer> concept
    void write(record_view record);

    //!doc: see record_writer_c<writer> concept
    void close();
};

static_assert(record_writer_c<writer>);

}
