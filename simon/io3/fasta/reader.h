#pragma once

#include "record.h"

#include <filesystem>
#include <functional>
#include <istream>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace io3::fasta {

template <typename reader>
struct reader_iter {
    using record_view = typename reader::record_view;

    reader& reader_;
    std::optional<record_view> nextItem = reader_.next();

    auto operator*() const -> record_view {
       return *nextItem;
    }
    auto operator++() -> reader_iter& {
        nextItem = reader_.next();
        return *this;
    }
    auto operator!=(std::nullptr_t) const {
        return nextItem.has_value();
    }

};
struct reader {
    struct pimpl;
    using record_view = fasta::record_view;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

        // This is only relevant if a stream is being used
        bool compressed{};
    };

private:
    std::unique_ptr<struct pimpl> pimpl_;

public:
    reader(config const& config_);
    ~reader();

    auto next() -> std::optional<record_view>;

    friend auto begin(reader& reader_) -> reader_iter<reader>;
    friend auto end(reader&) {
        return nullptr;
    }
};

}
