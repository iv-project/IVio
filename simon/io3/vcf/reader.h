#pragma once

#include "record.h"

#include "../buffered_reader.h"

#include <memory>
#include <optional>
#include <tuple>
#include <vector>

namespace io3::vcf {

struct reader_pimpl;
struct reader {
    struct iter;
private:
    std::unique_ptr<reader_pimpl> pimpl;

public:
    std::vector<std::tuple<std::string, std::string>> header;
    std::vector<std::string> genotypes;

    reader(VarBufferedReader reader);
    reader(reader const&) = delete;
    reader(reader&& _other) noexcept = default;
    ~reader();


    friend auto begin(reader& _reader) -> iter;
    friend auto end(reader const&) {
        return nullptr;
    }

    bool readHeaderLine();
    void readHeader();

    auto next() -> std::optional<record_view>;
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
