#pragma once

#include "../buffered_reader.h"
#include "../file_reader.h"

#include <charconv>
#include <functional>
#include <optional>
#include <ranges>

namespace io3::vcf {

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

struct iter {
    std::function<std::optional<record_view>()> next;
    std::optional<record_view> nextItem = next();

    auto operator*() const -> record_view {
       return *nextItem;
    }
    auto operator++() -> iter& {
        nextItem = next();
        return *this;
    }
    auto operator!=(std::nullptr_t) const {
        return nextItem.has_value();
    }
};


template <reader_and_dropper_c Reader>
struct reader_impl {
    Reader reader;
    size_t lastUsed{};

    template <typename R>
    reader_impl(R&& r)
        : reader{std::move(r)}
    {
        readHeader();
    }

    reader_impl(reader_impl const&) = delete;
    reader_impl(reader_impl&& _other) noexcept = default;
    ~reader_impl() = default;


    friend auto begin(reader_impl& reader) {
        return iter{[&reader]() { return reader.next(); }};
    }
    friend auto end(reader_impl const&) {
        return nullptr;
    }

    std::vector<std::tuple<std::string, std::string>> header;
    std::vector<std::string> genotypes;

    bool readHeaderLine() {
        auto [buffer, size] = reader.read(2);
        if (size >= 2 and buffer[0] == '#' and buffer[1] == '#') {
            auto start = 2;
            auto mid = reader.readUntil('=', start);
            if (reader.eof(mid)) return false;
            auto end = reader.readUntil('\n', mid+1);
            header.emplace_back(reader.string_view(start, mid), reader.string_view(mid+1, end));
            if (reader.eof(end)) return false;
            reader.dropUntil(end+1);
            return true;
        }
        return false;
    }

    void readHeader() {
        while (readHeaderLine()) {}
        auto [buffer, size] = reader.read(1);
        if (size >= 1 and buffer[0] == '#') {
            auto start = 1;
            auto end = reader.readUntil('\n', start);
            auto tableHeader = reader.string_view(start, end);
            for (auto v : std::views::split(tableHeader, '\t')) {
                genotypes.emplace_back(v.begin(), v.end());
            }
            if (genotypes.size() < 9) {
                throw std::runtime_error("Header description line is invalid");
            }
            genotypes.erase(begin(genotypes), begin(genotypes)+9);
            reader.dropUntil(end);
            if (!reader.eof(end)) reader.dropUntil(1);
        }
    }

    template <typename T>
    static auto convertTo(Reader& reader, std::string_view view) {
        T value;
        auto result = std::from_chars(begin(view), end(view), value);
        if (result.ec == std::errc::invalid_argument) {
            throw "can't convert to int";
        }
        return value;
    }

    struct {
        std::vector<std::string_view> alts;
        std::vector<std::string_view> filters;
        std::vector<std::string_view> infos;
        std::vector<std::string_view> formats;
        std::vector<std::string_view> samples_fields;
        std::vector<std::span<std::string_view>> samples;
    } storage;

    template <size_t ct, char sep>
    auto readLine() -> std::optional<std::array<std::string_view, ct>> {
        auto res = std::array<std::string_view, ct>{};
        size_t start{};
        for (size_t i{}; i < ct-1; ++i) {
            auto end = reader.readUntil(sep, start);
            if (reader.eof(end)) return std::nullopt;
            res[i] = reader.string_view(start, end);
            start = end+1;
        }
        auto end = reader.readUntil('\n', start);
        if (reader.eof(end)) return std::nullopt;
        res.back() = reader.string_view(start, end);
        lastUsed = end;
        if (!reader.eof(lastUsed)) lastUsed += 1;
        return res;
    }

    auto next() -> std::optional<record_view> {
        if (reader.eof(lastUsed)) return std::nullopt;
        reader.dropUntil(lastUsed);

        auto res = readLine<10, '\t'>();
        if (!res) return std::nullopt;

        auto [chrom, pos, id, ref, alt, qual, filters, infos, formats, samples] = *res;


        auto clearAndSplit = [&](std::vector<std::string_view>& targetVec, std::string_view str, char d) {
            targetVec.clear();
            for (auto && v : std::views::split(str, d)) {
                targetVec.emplace_back(v.begin(), v.end());
            }
        };

        clearAndSplit(storage.alts, alt, ',');
        clearAndSplit(storage.filters, filters, ';');
        if (filters == ".") storage.filters.clear();

        clearAndSplit(storage.infos, infos, ';');
        if(infos == ".") storage.infos.clear();

        clearAndSplit(storage.formats, formats, ':');

        storage.samples_fields.clear();
        auto field_groups = std::vector<size_t>{0};
        for (auto v : std::views::split(samples, '\t')) {
            for (auto v2 : std::views::split(v, ':')) {
                storage.samples_fields.emplace_back(v2.begin(), v2.end());
            }
            field_groups.emplace_back(ssize(storage.samples_fields));
        }

        storage.samples.clear();
        for (auto i{1}; i < ssize(field_groups); ++i) {
            auto iter = begin(storage.samples_fields);
            storage.samples.emplace_back(iter + field_groups[i-1], iter + field_groups[i]);
        }


        return record_view {
            .chrom   = chrom,
            .pos     = convertTo<int32_t>(reader, pos),
            .id      = id,
            .ref     = ref,
            .alt     = storage.alts,
            .qual    = convertTo<float>(reader, qual),
            .filter  = storage.filters,
            .info    = storage.infos,
            .formats = storage.formats,
            .samples = storage.samples,
        };
    }
};

template <typename Reader>
reader_impl(Reader&& reader) -> reader_impl<io3::buffered_reader<Reader>>;

template <reader_and_dropper_c Reader>
reader_impl(Reader&& reader) -> reader_impl<Reader>;

template <typename Reader>
using reader = reader_impl<Reader>;

static_assert(record_reader_c<reader<io3::buffered_reader<io3::file_reader>>>);
}
