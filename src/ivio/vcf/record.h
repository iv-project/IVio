#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace ivio::vcf {

struct record_view {
    std::string_view            chrom;
    int32_t                     pos;
    std::string_view            id;
    std::string_view            ref;
    std::string_view            alts;
    std::optional<float>        qual;
    std::string_view            filters;
    std::string_view            infos;
    std::string_view            formats;
    std::string_view            samples;
};

struct record {
    std::string                 chrom;
    int32_t                     pos;
    std::string                 id;
    std::string                 ref;
    std::string                 alts;
    std::optional<float>        qual;
    std::string                 filters;
    std::string                 infos;
    std::string                 formats;
    std::string                 samples;

    record() = default;
    record(record_view v)
        : chrom  {v.chrom}
        , pos    {v.pos}
        , id     {v.id}
        , ref    {v.ref}
        , alts   {v.alts}
        , qual   {v.qual}
        , filters{v.filters}
        , infos  {v.infos}
        , formats{v.formats}
        , samples{v.samples}
    {}
    operator record_view() const {
        return record_view {
            chrom,
            pos,
            id,
            ref,
            alts,
            qual,
            filters,
            infos,
            formats,
            samples,
        };
    }
};

}
