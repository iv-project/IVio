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
struct reader_base {
    struct iter {
        using record_view = typename reader::record_view;

        reader& reader_;
        std::optional<record_view> nextItem = reader_.next();

        auto operator*() const -> record_view {
           return *nextItem;
        }
        auto operator++() -> iter& {
            nextItem = reader_.next();
            return *this;
        }
        auto operator!=(std::nullptr_t) const {
            return nextItem.has_value();
        }
    };

protected:
    struct pimpl;
    std::unique_ptr<pimpl> pimpl_;
public:
    reader_base(std::unique_ptr<pimpl> pimpl_)
        : pimpl_{std::move(pimpl_)}
    {}
    friend auto begin(reader& reader_) {
        return iter{reader_};
    }
    friend auto end(reader&) {
        return nullptr;
    }
};
struct reader : public reader_base<reader> {
    using record_view = fasta::record_view;

    struct config {
        // Source: file or stream
        std::variant<std::filesystem::path, std::reference_wrapper<std::istream>> input;

        // This is only relevant if a stream is being used
        bool compressed{};
    };

public:
    reader(config const& config_);
    ~reader();

    auto next() -> std::optional<record_view>;
};

}
