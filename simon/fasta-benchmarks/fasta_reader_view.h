#pragma once

#include <functional>

inline constexpr auto seq_cleanuped_view = std::views::transform([](char c) {
            return ccmap[reinterpret_cast<uint8_t&>(c)];
        }) | std::views::filter([](char c) {
            if (c == (char)0xfe) {
                throw "invalid variable";
            }
            return c != (char)0xff;
        });

struct fasta_reader_view_record_view {
    using seq_view = decltype(std::string_view{} | seq_cleanuped_view);

    std::string_view id;
    seq_view seq;
};

struct fasta_reader_view_iter {
    using record_view = fasta_reader_view_record_view;
    std::function<std::optional<record_view>()> next;
    std::optional<record_view> nextItem = next();

    auto operator*() const -> record_view {
       return *nextItem;
    }
    auto operator++() -> fasta_reader_view_iter& {
        nextItem = next();
        return *this;
    }
    auto operator!=(std::nullptr_t _end) const {
        return nextItem.has_value();
    }
};


template <typename Reader>
struct fasta_reader_view {
    Reader file;

    size_t lastUsed{};
    std::vector<char> buf = []() { auto vec = std::vector<char>{}; vec.reserve(4096); return vec; }();

    using record_view = fasta_reader_view_record_view;
    using iter = fasta_reader_view_iter;

    fasta_reader_view(Reader&& reader)
        : file{std::move(reader)}
    {}

    fasta_reader_view(fasta_reader_view const&) = delete;
    fasta_reader_view(fasta_reader_view&& _other) noexcept = default;
    ~fasta_reader_view() = default;



    friend auto begin(fasta_reader_view& reader) {
        return iter{[&reader]() { return reader.next(); }};
    }
    friend auto end(fasta_reader_view const&) {
        return nullptr;
    }
    auto readMore() {
        size_t lastSize = buf.size();
        if (buf.capacity() - buf.size() >= 4*1024ul) {
            buf.resize(buf.capacity());
        } else {
            buf.resize(buf.capacity()*2);
        }

        auto bytes_read = file.read(std::ranges::subrange{buf.data() + lastSize, &*buf.end()});
        buf.resize(lastSize + bytes_read);

        return bytes_read != 0;
    }
    size_t readUntil(char c) {
        while (true) {
            auto ptr = (char const*)memchr(buf.data() + lastUsed, c, buf.size() - lastUsed);
            if (ptr != nullptr) {
                lastUsed = ptr - buf.data();
                return lastUsed;
            }
            if (!readMore()) {
                return buf.size();
            }
        }
    }

    auto next() -> std::optional<record_view> {
        auto startId = readUntil('>');
        if (startId == buf.size()) return std::nullopt;
        buf.erase(begin(buf), begin(buf)+startId+1);
        lastUsed = 0;
        startId = 0;

        auto endId = readUntil('\n');
        if (endId == buf.size()) return std::nullopt;

        auto startSeq = endId+1;
        auto endSeq = readUntil('>');
        if (endSeq != buf.size()) {
            lastUsed -= 1;
        }

        return record_view{
            .id  = std::string_view{buf.data()+startId, buf.data()+endId},
            .seq = std::string_view{buf.data()+startSeq, buf.data() + endSeq} | seq_cleanuped_view};
    }
};

