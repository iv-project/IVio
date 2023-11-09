// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string>
#include <string_view>

namespace ivio::fastq {

struct record_view {
    std::string_view id;
    std::string_view seq;
    std::string_view id2;
    std::string_view qual;
};

struct record {
    std::string id;
    std::string seq;
    std::string id2;
    std::string qual;

    record() = default;
    record(record_view v)
        : id   {v.id}
        , seq  {v.seq}
        , id2  {v.id2}
        , qual {v.qual}
    {}

    operator record_view() const {
        return record_view {
            id,
            seq,
            id2,
            qual
        };
    }
};

}
