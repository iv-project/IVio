// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "../detail/concepts.h"
#include "../detail/reader_base.h"
#include "header.h"
#include "record.h"

#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>

namespace ivio::vcf {

struct reader : public reader_base<reader> {
    using record      = vcf::record;      //!doc: see record_reader_c<reader> concept
    using record_view = vcf::record_view; //!doc: see record_reader_c<reader> concept

    vcf::header header_;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;
    };

public:
    reader(config const& config_);
    ~reader();

    auto header() const -> vcf::header const& { return header_; }

    //!doc: see record_reader_c<reader> concept
    auto next() -> std::optional<record_view>;

    //!doc: see record_reader_c<reader> concept
    void close();
};

static_assert(record_reader_c<reader>);

}
