#pragma once

#include <optional>

namespace io2::detail {
/** Iterator that enables iterating over the reader
 */
template <typename Reader, typename Record>
struct iterator {
    Reader& reader;
    std::optional<Record> nextItem = reader.next();

    auto operator*() const -> Record {
       return *nextItem;
    }
    auto operator++() -> iterator& {
        nextItem = reader.next();
        return *this;
    }
    auto operator!=(std::nullptr_t _end) const {
        return nextItem.has_value();
    }
};

}
