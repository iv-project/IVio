#pragma once

#include <seqan/seq_io.h>
#include <filesystem>
#include <ranges>
#include <optional>
#include <functional>
#include <seqan3/alphabet/nucleotide/dna5.hpp>

namespace io2 {

namespace detail {

// a empty base class
template <typename T>
struct empty_class {
    empty_class() = default;
    empty_class(T const& t) {}
};


// helper function to count the number of bits to represents the given number of states
constexpr unsigned numberOfBits(unsigned nrStates) {
    return nrStates < 2 ? nrStates : 1+numberOfBits(nrStates >> 1);
}

// Alphabet Adaptor, to map seqan3 alphabet to seqan2 alphabet
template <typename AlphabetS3>
struct AlphabetAdaptor_ {};

template <typename AlphabetS3>
using AlphabetAdaptor = seqan::SimpleType<uint8_t, AlphabetAdaptor_<AlphabetS3>>;

// interfere conversion tables from seqan3 alphabets
template <typename AlphabetS3>
struct tables {
    static constexpr std::array<uint8_t, 256> toRank = []() {
        auto map = std::array<uint8_t, 256>{};
        for (size_t i{0}; i<map.size(); ++i) {
            map[i] = seqan3::assign_char_to((char)i, AlphabetS3{}).to_rank();
        }
        return map;
    }();


    static constexpr std::array<uint8_t, AlphabetS3::alphabet_size> toChar = []() {
        auto map = std::array<uint8_t, AlphabetS3::alphabet_size>{};
        for (size_t i{0}; i < map.size(); ++i) {
            map[i] = seqan3::assign_rank_to(i, AlphabetS3{}).to_char();
        }
        return map;
    }();
};
}
}


template <typename AlphabetS3>
struct seqan::ValueSize<io2::detail::AlphabetAdaptor<AlphabetS3>> {
    using Type = uint8_t;
    static constexpr Type VALUE = io2::detail::AlphabetAdaptor<AlphabetS3>::alphabet_size;
};

template <typename AlphabetS3>
struct seqan::BitsPerValue<io2::detail::AlphabetAdaptor<AlphabetS3>> {
    using Type = uint8_t;
    static constexpr Type VALUE = numberOfBits(io2::detail::AlphabetAdaptor<AlphabetS3>::alphabet_size);
};


namespace seqan {

template <typename AlphabetS3>
inline io2::detail::AlphabetAdaptor<AlphabetS3> unknownValueImpl(io2::detail::AlphabetAdaptor<AlphabetS3>*) {
    return io2::detail::AlphabetAdaptor<AlphabetS3>('N');
}

template <typename AlphabetS3>
constexpr inline void assign(io2::detail::AlphabetAdaptor<AlphabetS3> & target, char c_source) {
    target.value = io2::detail::tables<AlphabetS3>::toRank[(size_t) c_source];
}

template <typename AlphabetS3>
constexpr inline void assign(char & c_target, io2::detail::AlphabetAdaptor<AlphabetS3> const & source) {
    c_target = io2::detail::tables<AlphabetS3>::toChar[(size_t)source.value];
}

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
