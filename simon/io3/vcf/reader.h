#pragma once

#include "record.h"

#include "../buffered_reader.h"
#include "../file_reader.h"

#include <charconv>
#include <functional>
#include <optional>
#include <ranges>

namespace io3::vcf {

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


struct reader {
    VarBufferedReader _reader;
    size_t lastUsed{};

    reader(VarBufferedReader reader)
        : _reader{std::move(reader)}
    {
        readHeader();
    }

    reader(reader const&) = delete;
    reader(reader&& _other) noexcept = default;
    ~reader() = default;


    friend auto begin(reader& _reader) {
        return iter{[&_reader]() { return _reader.next(); }};
    }
    friend auto end(reader const&) {
        return nullptr;
    }

    std::vector<std::tuple<std::string, std::string>> header;
    std::vector<std::string> genotypes;

    bool readHeaderLine() {
        auto [buffer, size] = _reader.read(2);
        if (size >= 2 and buffer[0] == '#' and buffer[1] == '#') {
            auto start = 2;
            auto mid = _reader.readUntil('=', start);
            if (_reader.eof(mid)) return false;
            auto end = _reader.readUntil('\n', mid+1);
            header.emplace_back(_reader.string_view(start, mid), _reader.string_view(mid+1, end));
            if (_reader.eof(end)) return false;
            _reader.dropUntil(end+1);
            return true;
        }
        return false;
    }

    void readHeader() {
        while (readHeaderLine()) {}
        auto [buffer, size] = _reader.read(1);
        if (size >= 1 and buffer[0] == '#') {
            auto start = 1;
            auto end = _reader.readUntil('\n', start);
            auto tableHeader = _reader.string_view(start, end);
            for (auto v : std::views::split(tableHeader, '\t')) {
                genotypes.emplace_back(v.begin(), v.end());
            }
            if (genotypes.size() < 9) {
                throw std::runtime_error("Header description line is invalid");
            }
            genotypes.erase(begin(genotypes), begin(genotypes)+9);
            _reader.dropUntil(end);
            if (!_reader.eof(end)) _reader.dropUntil(1);
        }
    }

    template <typename T>
    static auto convertTo(std::string_view view) {
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
            auto end = _reader.readUntil(sep, start);
            if (_reader.eof(end)) return std::nullopt;
            res[i] = _reader.string_view(start, end);
            start = end+1;
        }
        auto end = _reader.readUntil('\n', start);
        if (_reader.eof(end)) return std::nullopt;
        res.back() = _reader.string_view(start, end);
        lastUsed = end;
        if (!_reader.eof(lastUsed)) lastUsed += 1;
        return res;
    }

    auto next() -> std::optional<record_view> {
        if (_reader.eof(lastUsed)) return std::nullopt;
        _reader.dropUntil(lastUsed);

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
            .pos     = convertTo<int32_t>(pos),
            .id      = id,
            .ref     = ref,
            .alt     = storage.alts,
            .qual    = convertTo<float>(qual),
            .filter  = storage.filters,
            .info    = storage.infos,
            .formats = storage.formats,
            .samples = storage.samples,
        };
    }
};

static_assert(record_reader_c<reader>);
}
