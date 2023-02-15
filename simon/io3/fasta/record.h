#pragma once

#include <string>
#include <string_view>

namespace io3::fasta {

struct record_view {
    std::string_view id;
    std::string_view seq;
};

struct record {
    std::string id;
    std::string seq;

    record() = default;
    record(record_view v)
        : id{v.id}
        , seq{v.seq}
    {}

    operator record_view() const {
        return record_view{id, seq};
    }
};

}
