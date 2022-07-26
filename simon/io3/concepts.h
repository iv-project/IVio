#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <vector>

namespace io3 {

template <typename T>
concept reader_c = requires(T t) {
    { t.read(std::declval<std::vector<int>&>()) } -> std::same_as<size_t>;
};

template <typename T>
concept reader_and_dropper_c = requires(T t) {
    { t.readUntil(char{}, size_t{}) } -> std::same_as<size_t>;
    { t.dropUntil(size_t{}) } -> std::same_as<void>;
    { t.eof(size_t{}) } -> std::same_as<bool>;
    { t.string_view(size_t{}, size_t{}) } -> std::same_as<std::string_view>;
};

template <typename T>
concept record_reader_c = requires(T t) {
    { t.next() };
};

}
