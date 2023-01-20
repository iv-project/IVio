#pragma once


template <typename Reader>
struct fasta_reader_contigous {
    Reader reader;

    size_t lastUsed{};
    std::vector<char> buf = []() { auto vec = std::vector<char>{}; vec.reserve(4096); return vec; }();

    struct record_view {
        std::string_view id;
        std::string_view seq;
    };

    struct iter {
        fasta_reader_contigous& reader;
        std::optional<record_view> nextItem = reader.next();

        auto operator*() const -> record_view {
           return *nextItem;
        }
        auto operator++() -> iter& {
            nextItem = reader.next();
            return *this;
        }
        auto operator!=(std::nullptr_t _end) const {
            return nextItem.has_value();
        }
    };

    friend auto begin(fasta_reader_contigous& reader) {
        return iter{reader};
    }
    friend auto end(fasta_reader_contigous const&) {
        return nullptr;
    }
    auto readMore() {
        size_t lastSize = buf.size();
        if (buf.capacity() - buf.size() >= 4*1024ul) {
            buf.resize(buf.capacity());
        } else {
            buf.resize(buf.capacity()*2);
        }
        auto bytes_read = reader.read(std::ranges::subrange{buf.data() + lastSize, &*buf.end()});
        buf.resize(lastSize + bytes_read);
        return bytes_read != 0;
    }
    size_t readUntil(char c) {
        while (true) {
            auto ptr = (char*)memchr(buf.data() + lastUsed, c, buf.size() - lastUsed);
            if (ptr != nullptr) {
                lastUsed = ptr - buf.data();
                return lastUsed;
            }
            if (!readMore()) {
                return buf.size();
            }
        }
    }

    size_t compact(size_t start, size_t end) {
        auto p1 = buf.data()+start;
        auto p2 = buf.data()+start;
        auto pend = buf.data()+end;

        // convert char and to rank and move forward if valid
        for (;p2 != pend; ++p2) {
            auto c = ccmap[*p2];
            [[likely]]
            if (c != (char)0xff) {
                *p1 = c;
                ++p1;
            }
        }
        return std::distance(buf.data(), p1);
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

        endSeq = compact(startSeq, endSeq);
        return record_view{
            .id  = std::string_view{buf.data()+startId, buf.data()+endId},
            .seq = std::string_view{buf.data()+startSeq, buf.data() + endSeq}};
    }
};

