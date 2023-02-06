#pragma once

#include <memory>
#include <optional>

namespace io3 {

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

}
