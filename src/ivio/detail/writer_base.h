// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <memory>
#include <optional>

namespace ivio {

template <typename writer>
struct writer_base {
protected:
    struct pimpl;
    std::unique_ptr<pimpl> pimpl_;
public:
    writer_base(std::unique_ptr<pimpl> pimpl_)
        : pimpl_{std::move(pimpl_)}
    {}
};

}
