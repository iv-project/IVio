#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace ivio::vcf {

struct record_view {
    using string_view_list = std::span<std::string_view>;

    std::string_view            chrom;
    int32_t                     pos;
    std::string_view            id;
    std::string_view            ref;
    string_view_list            alt;
    std::optional<float>        qual;
    string_view_list            filter;
    string_view_list            info;
    string_view_list            formats;
    std::span<string_view_list> samples;
};

struct record {
    std::string                 chrom;
    int32_t                     pos;
    std::string                 id;
    std::string                 ref;
    std::vector<std::string>    alt;
    std::optional<float>        qual;
    std::vector<std::string>    filter;
    std::vector<std::string>    info;
    std::vector<std::string>    formats;
    std::vector<std::vector<std::string>> samples;

    record() = default;
    record(record_view v)
        : chrom {v.chrom}
        , pos   {v.pos}
        , id    {v.id}
        , ref   {v.ref}
        , alt   {begin(v.alt), end(v.alt)}
        , qual  {v.qual}
        , filter{begin(v.filter), end(v.filter)}
        , info  {begin(v.info), end(v.info)}
        , formats{begin(v.info), end(v.info)}
        , samples{} //!TODO
    {}
    operator record_view() const {
        return record_view {
            chrom,
            pos,
            id,
            ref,
            {}, //!TODO alt
            qual,
            {}, //!TODO filter
            {}, //!TODO info
            {}, //!TODO formats
            {}, //!TODO samples
        };
    }
};

}
