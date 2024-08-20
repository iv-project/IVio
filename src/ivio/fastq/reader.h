// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "../detail/concepts.h"
#include "../detail/reader_base.h"
#include "record.h"

#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>

namespace ivio::fastq {

struct reader : public reader_base<reader> {
    using record      = fastq::record;
    using record_view = fastq::record_view;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;
    };

public:
    reader(config const& config_);
    ~reader();

    //!doc: see record_reader_c<reader> concept
    auto next() -> std::optional<record_view>;

    //!doc: see record_reader_c<reader> concept
    void close();
};

static_assert(record_reader_c<reader>);

}
