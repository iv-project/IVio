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

namespace ivio::fasta {

struct reader : public reader_base<reader> {
    using record      = fasta::record;
    using record_view = fasta::record_view;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;
    };

public:
    reader(config const& config_);
    ~reader();

    /**
     * Reads next record
     *
     * \return returns the next record_view if available
     *         otherwise std::nullopt
     */
    auto next() -> std::optional<record_view>;

    /**
     * Closes the underlying stream/file handler
     */
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

    /**
     * Report the last read record and its FAIDX index
     */
    auto tell_faidx() const -> faidx::record;

    /**
     * Seek to a precise record according to an FAIDX index
     */
    void seek_faidx(faidx::record const& offset);
};

static_assert(record_reader_c<reader>);

}
