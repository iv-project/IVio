#pragma once

#include <memory>
#include <optional>

namespace io3 {

template <typename writer>
struct writer_base {
protected:
    struct pimpl;
    std::unique_ptr<pimpl> pimpl_;
public:
    writer_base(std::unique_ptr<pimpl> pimpl_)
        : pimpl_{std::move(pimpl_)}
    {}
};

}
