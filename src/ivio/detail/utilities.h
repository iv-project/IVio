// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#pragma once

#include <string_view>
#include <sstream>
#include <charconv>

namespace ivio::detail {

template <typename T>
static auto convertTo(std::string_view view) {
    T value{}; //!Should not be initialized with {}, but gcc warns...
    //!WORKAROUND missing std::from_chars in msvc
    #if defined(_WIN32) || defined(__clang__)
    std::stringstream ss;
    ss << view;
    ss >> value;
    if (!ss.eof() || ss.fail()) {
        throw std::runtime_error{"can't convert to T"};
    }
    #else
    auto result = std::from_chars(begin(view), end(view), value);
    if (result.ec == std::errc::invalid_argument) {
        throw std::runtime_error{"can't convert to T"};
    }
    #endif
    return value;
}

}
