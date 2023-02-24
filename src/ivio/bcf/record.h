#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace ivio::bcf {

struct record_view {
    using string_view_list = std::span<std::string_view>;

    int32_t                  chromId;
    int32_t                  pos;
    std::string_view         id;
    std::string_view         ref;
    int32_t                  n_allele;
    std::span<uint8_t const> alts;
    std::optional<float>     qual;
    std::span<int32_t const> filters;
    std::string_view         info;
    std::string_view         format;
    std::string_view         samples;
};

struct record {
    int32_t                  chromId;
    int32_t                  pos;
    std::string              id;
    std::string              ref;
    int32_t                  n_allele;
    std::vector<uint8_t>     alts;
    std::optional<float>     qual;
    std::vector<int32_t>     filters;
    std::string              info;
    std::string              format;
    std::string              samples;

    record() = default;
    record(record_view v)
        : chromId {v.chromId}
        , pos     {v.pos}
        , id      {v.id}
        , ref     {v.ref}
        , n_allele{v.n_allele}
        , alts    {begin(v.alts), end(v.alts)}
        , qual    {v.qual}
        , filters {begin(v.filters), end(v.filters)}
        , info    {v.info}
        , format  {v.format}
        , samples {v.samples}
    {}
    operator record_view() const {
        return record_view {
            chromId,
            pos,
            id,
            ref,
            n_allele,
            alts,
            qual,
            filters,
            info,
            format,
            samples,
        };
    }
};


}
