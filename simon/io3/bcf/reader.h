#pragma once

#include "record.h"

#include "../buffered_reader.h"

#include "../bgzf_reader.h"
#include "../file_reader.h"

#include <functional>
#include <unordered_map>

namespace io3::bcf {

struct reader_pimpl;
struct reader {
    struct iter;
private:
    std::unique_ptr<reader_pimpl> pimpl;

public:
    std::string headerBuffer;
    std::vector<std::string_view> header;
    std::string_view              tableHeader;
    std::unordered_map<std::string_view, std::vector<std::string_view>> headerMap;

    std::vector<std::string_view>& contigMap = headerMap["contig"];
    std::vector<std::string_view>& filterMap = headerMap["filter"];


    reader(VarBufferedReader r);
    reader(reader const&) = delete;
    reader(reader&& _other) noexcept = default;
    ~reader();

    auto next() -> std::optional<record_view>;

    friend auto begin(reader& _reader) -> iter;
    friend auto end(reader const&) {
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




static_assert(record_reader_c<reader>);
}
