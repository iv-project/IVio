// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include "../detail/concepts.h"
#include "../detail/reader_base.h"
#include "../faidx/record.h"
#include "record.h"

#include <filesystem>
#include <optional>
#include <tuple>
#include <variant>

namespace ivio::csv {

struct reader : public reader_base<reader> {
    using record      = csv::record;      //!doc: see record_reader_c<reader> concept
    using record_view = csv::record_view; //!doc: see record_reader_c<reader> concept

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;
        char delimiter{','};
        bool trim{false}; // trim whitespaces
    };

public:
    reader(config const& config_);
    ~reader();

    //!doc: see record_reader_c<reader> concept
    auto next() -> std::optional<record_view>;

    //!doc: see record_reader_c<reader> concept
    void close();

    /**
     * Reports the current offset inside the file
     * \return offset of file
     */
    auto tell() const -> size_t;

    /**
     * Moves the current offset to a certain position
     */
    void seek(size_t offset);
};

static_assert(record_reader_c<reader>);

}
