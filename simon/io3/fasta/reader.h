#pragma once

#include "meta.h"

#include <filesystem>
#include <functional>
#include <istream>
#include <memory>
#include <optional>
#include <string_view>
#include <variant>

namespace io3::fasta {

struct reader_config {
    // Source: file or stream
    std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

    // This is only relevant if a stream is being used
    bool compressed{};
};

struct reader_pimpl;
struct reader {
    using record_view = fasta::record_view;
    struct iter;

private:
    std::unique_ptr<reader_pimpl> pimpl;

public:
    reader(reader_config config);
    ~reader();

    auto next() -> std::optional<record_view>;

    friend auto begin(reader& reader) -> iter;
    friend auto end(reader& reader) {
        return nullptr;
    }
};

struct reader::iter {
    reader& _reader;
    std::optional<record_view> nextItem = _reader.next();

    auto operator*() const -> record_view {
       return *nextItem;
    }
    auto operator++() -> iter& {
        nextItem = _reader.next();
        return *this;
    }
    auto operator!=(std::nullptr_t) const {
        return nextItem.has_value();
    }
};


}
