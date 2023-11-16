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

    //!WORKAROUND std::from_chars for float/double is missing from libc++
    // libc++ does not define __cpp_lib_to_chars
    #if defined(_LIBCPP_VERSION) && !defined(__cpp_lib_to_chars)
    if constexpr (std::floating_point<T>)
    #else
    if constexpr (false)
    #endif
    {
        std::stringstream ss;
        ss << view;
        ss >> value;
        if (!ss.eof() || ss.fail()) {
            throw std::runtime_error{"can't convert to T"};
        }
    }
    else
    {
        auto result = std::from_chars(view.data(), view.data() + view.size(), value);
        if (result.ec == std::errc::invalid_argument) {
            throw std::runtime_error{"can't convert to T"};
        }
    }

    return value;
}

}
