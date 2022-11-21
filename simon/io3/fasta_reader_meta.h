#pragma once

#include <functional>
#include <optional>
#include <string_view>

namespace io3 {

struct fasta_reader_view_record_view {
    std::string_view id;
    std::string_view seq;
};

struct fasta_reader_view_iter {
    using record_view = fasta_reader_view_record_view;
    std::function<std::optional<record_view>()> next;
    std::optional<record_view> nextItem = next();

    auto operator*() const -> record_view {
       return *nextItem;
    }
    auto operator++() -> fasta_reader_view_iter& {
        nextItem = next();
        return *this;
    }
    auto operator!=(std::nullptr_t) const {
        return nextItem.has_value();
    }
};

}
