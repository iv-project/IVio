#pragma once

#include "alphabet_seqan223.h"

#include <ranges>
#include <seqan/seq_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <string_view>


namespace io2::detail {

/** An empty base class for type deduction.
 * \noapi
 */
template <typename T>
struct empty_class {
    empty_class() = default;
    empty_class(T const &) {}
};

}

namespace io2 {

// a helper variable to declare types, see call of io2::seq_io::reader
template <typename T>
inline constexpr auto type = detail::empty_class<T>{};

template <typename T>
inline auto to_view(seqan::String<T> const& v) {
    auto ptr = &*begin(v, seqan::Standard());
    return std::ranges::subrange{ptr, ptr + length(v)};
}

inline auto to_view(seqan::String<char> const& v) {
    return std::string_view{&*begin(v, seqan::Standard()), length(v)};
}


template <typename AlphabetS3>
inline auto toSeqan3(seqan::String<detail::AlphabetAdaptor<AlphabetS3>> const& v) {
    return to_view(v) | std::views::transform([](auto const& v) {
        return seqan3::assign_rank_to(v.value, AlphabetS3{});
    });
}

template <typename AlphabetS3, typename T>
auto toSeqan3(seqan::String<T> const& v) {
    return to_view(v) | std::views::transform([](auto const& v) {
        return seqan3::assign_char_to(static_cast<char>(v), AlphabetS3{});
    });
}

}
