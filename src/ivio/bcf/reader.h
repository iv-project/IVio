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
#include <unordered_map>

namespace ivio::bcf {

struct reader : public reader_base<reader> {
    using record      = bcf::record;
    using record_view = bcf::record_view;

    bcf::header header_;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

        // Value of 0 will run with a sequential implementation, other values will spawn new threads
        size_t threadNbr = 0;
    };

public:
    reader(config const& config_);
    ~reader();

    auto header() const -> bcf::header const& { return header_; }
    auto next() -> std::optional<record_view>;
    void close();
};

static_assert(record_reader_c<reader>);

}
