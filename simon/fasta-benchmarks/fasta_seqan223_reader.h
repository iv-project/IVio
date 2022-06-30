#pragma once

#include <seqan/seq_io.h>
#include <filesystem>
#include <ranges>
#include <optional>
#include <functional>
#include <seqan3/alphabet/nucleotide/dna5.hpp>

namespace sgg_io {

template <typename T>
struct empty_class {
    empty_class() = default;
    empty_class(T const& t) {}
};

template <typename T>
inline constexpr auto type = empty_class<T>{};

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

template <typename AlphabetS3>
struct seqan::ValueSize<sgg_io::AlphabetAdaptor<AlphabetS3>> {
    using Type = uint8_t;
    static constexpr Type VALUE = sgg_io::AlphabetAdaptor<AlphabetS3>::alphabet_size;
};

template <typename AlphabetS3>
struct seqan::BitsPerValue<sgg_io::AlphabetAdaptor<AlphabetS3>> {
    using Type = uint8_t;
    static constexpr Type VALUE = numberOfBits(sgg_io::AlphabetAdaptor<AlphabetS3>::alphabet_size);
};


namespace seqan {

template <typename AlphabetS3>
inline sgg_io::AlphabetAdaptor<AlphabetS3> unknownValueImpl(sgg_io::AlphabetAdaptor<AlphabetS3>*) {
    return sgg_io::AlphabetAdaptor<AlphabetS3>('N');
}

template <typename AlphabetS3>
constexpr inline void assign(sgg_io::AlphabetAdaptor<AlphabetS3> & target, char c_source) {
    target.value = sgg_io::tables<AlphabetS3>::toRank[(size_t) c_source];
}

template <typename AlphabetS3>
constexpr inline void assign(char & c_target, sgg_io::AlphabetAdaptor<AlphabetS3> const & source) {
    c_target = sgg_io::tables<AlphabetS3>::toChar[(size_t)source.value];
}

}


namespace sgg_io {

namespace seq_io {

template <typename AlphabetS3>
inline auto toSeqan3(seqan::String<AlphabetAdaptor<AlphabetS3>> const& v) {
    return std::ranges::subrange{&v[0], &v[0] + length(v)} | std::views::transform([](auto const& v) {
        return reinterpret_cast<AlphabetS3 const&>(v);
    });
}


template <typename AlphabetS3>
using sequence_view = decltype(toSeqan3<AlphabetS3>({}));

//!TODO swap seq and id, this is just for demonstration/seqan3 compatbility, but not really needed
template <typename AlphabetS3>
struct record {
    sequence_view<AlphabetS3> seq;
    std::string_view          id;
    uint8_t                   qual; //!TODO
};

template <typename AlphabetS3 = seqan3::dna5>
struct reader {
    struct Input {
        std::string path;
        seqan::SeqFileIn fileIn{seqan::toCString(path)};

        Input(std::string const& _path)
            : path{_path}
        {}
        Input(std::filesystem::path const& _path)
            : path{_path.string()}
        {}
    };

    struct iter {
        std::function<std::optional<record<AlphabetS3>>()> next;
        std::optional<record<AlphabetS3>> nextItem = next();

        auto operator*() const -> record<AlphabetS3> {
           return *nextItem;
        }
        auto operator++() -> iter& {
            nextItem = next();
            return *this;
        }
        auto operator!=(std::nullptr_t _end) const {
            return nextItem.has_value();
        }
    };

    Input input;
    [[no_unique_address]] empty_class<AlphabetS3> alphabet{};

    struct {
        seqan::CharString id;
        seqan::String<AlphabetAdaptor<AlphabetS3>> seq;
    } storage;

    auto next() -> std::optional<record<AlphabetS3>> {
        if (atEnd(input.fileIn)) return std::nullopt;
        readRecord(storage.id, storage.seq, input.fileIn);

        return record<AlphabetS3> {
            .seq = toSeqan3(storage.seq),
            .id = std::string_view{&storage.id[0], &storage.id[0]+length(storage.id)},
        };
    }

    friend auto begin(reader& _reader) {
        return iter{[&_reader]() { return _reader.next(); }};
    }
    friend auto end(reader const&) {
        return nullptr;
    }

};

}
}
