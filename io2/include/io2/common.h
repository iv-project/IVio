#pragma once

#include "alphabet_seqan223.h"

#include <ranges>
#include <seqan/seq_io.h>
#include <seqan3/alphabet/nucleotide/dna5.hpp>
#include <seqan3/alphabet/cigar/cigar.hpp>
#include <string_view>


namespace io2::detail {

/** An empty base class for type deduction.
 *
 * It is meant to be used in combination with the attribute
 * [[no_unique_address]].
 * See io2::type for an example.
 *
 * \tparam t a captured type.
 *
 * \noapi
 */
template <typename t>
struct empty_class {
    using type = t;
    empty_class() = default;
    empty_class(t const &) {}
};



template <typename T>
auto convert_to_view(seqan::String<T> const& v) {
    auto ptr = &*begin(v, seqan::Standard());
    return std::ranges::subrange{ptr, ptr + length(v)};
}

inline auto convert_to_view(seqan::String<char> const& v) {
    return std::string_view{&*begin(v, seqan::Standard()), length(v)};
}


template <typename AlphabetS3>
auto convert_to_seqan3_view(seqan::String<detail::AlphabetAdaptor<AlphabetS3>> const& v) {
    return convert_to_view(v) | std::views::transform([](auto const& v) {
        return seqan3::assign_rank_to(v.value, AlphabetS3{});
    });
}

template <typename AlphabetS3, typename T>
auto convert_to_seqan3_view(seqan::String<T> const& v) {
    return convert_to_view(v) | std::views::transform([](auto const& v) {
        return seqan3::assign_char_to(static_cast<char>(v), AlphabetS3{});
    });
}

inline auto convert_to_seqan3_view(seqan::String<seqan::CigarElement<>> const& v) {
    return convert_to_view(v) | std::views::transform([](auto const& v) {
        using namespace seqan3::literals;
        seqan3::cigar::operation letter{};
        letter.assign_char(v.operation);
        return seqan3::cigar{v.count, letter};
    });
}

auto convert_to_seqan2_alphabet(std::ranges::range auto rng) {
    using AlphabetS3 = std::decay_t<decltype(*rng.begin())>;
    seqan::String<io2::detail::AlphabetAdaptor<AlphabetS3>> v;
    resize(v, size(rng), seqan::Exact());
    std::ranges::copy(rng | std::views::transform([](auto c) {
        auto t = io2::detail::AlphabetAdaptor<AlphabetS3>{};
        t.value = c.to_rank();
        return t;
    }), begin(v));
    return v;
}

auto convert_to_seqan2_qualities(std::ranges::range auto rng) {
    return convert_to_seqan2_alphabet(rng);
}

auto convert_to_seqan2_cigar(std::ranges::range auto rng) {
    seqan::String<seqan::CigarElement<>> v;
    resize(v, size(rng), seqan::Exact());
    std::ranges::copy(rng | std::views::transform([](auto c) {
        auto t = seqan::CigarElement{};
        t.operation = get<1>(c).to_char();
        t.count     = get<0>(c);
        return t;
    }), begin(v));
    return v;
}

auto convert_to_seqan2_string(std::ranges::range auto rng) {
    seqan::String<char> v;
    resize(v, size(rng));
    std::ranges::copy(rng, begin(v));
    return v;
}





}

namespace io2 {

/**\brief A helper variable to declare types
 *
 * \tparam t the type being deduced.
 *
 * for example:
 * ```
 * template <typename t>
 * struct A {
 *     [[no_unique_address]] io2::detail::empty_class<t> value_type;
 * };
 *
 * // This allows to declare `t` in two ways:
 * A<int> a{};
 *
 * // Or the fancy way:
 * A a {
 *  .value_type = io2::type<int>;
 * }
 * ```
 */
template <typename t>
inline constexpr auto type = detail::empty_class<t>{};


}
