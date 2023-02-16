#include "writer.h"

#include "../bgzf_writer.h"

#include <cassert>
#include <cstddef>
#include <variant>

namespace ivio::bcf {

namespace {
template <typename T>
inline auto bgzfPack(T v, char* buffer) -> size_t {
    return bgzf_writer::detail::bgzfPack(v, buffer);
}

struct bcf_buffer {
    std::vector<char> buffer;

    void clear() {
        buffer.clear();
    }

    template <typename T>
    void pack(T v) {
        auto oldSize = buffer.size();
        buffer.resize(oldSize + sizeof(std::decay_t<T>));
        bgzf_writer::detail::bgzfPack(v, buffer.data() + oldSize);
    }

    template <std::integral T>
    void writeInt(T v) {
        auto type = [&]() -> uint8_t {
            if constexpr (std::same_as<T, int8_t>) return 1;
            if constexpr (std::same_as<T, int16_t>) return 2;
            if constexpr (std::same_as<T, int32_t>) return 3;
            throw "BCF error, expected an int(2)";
        }();
        pack<uint8_t>(type);
        pack(v);
    };

    void writeString(std::string_view v) {
        if (v.size() < 15) { // No overflow
            auto l = (v.size() << 4) | 0x07;
            pack<uint8_t>(l);
        } else { // overflow
            pack<uint8_t>(0xf7);
            if (v.size() > 127) {
                throw "BCF: string to long";
            }
            writeInt<int8_t>(v.size());
        }
        auto oldSize = buffer.size();
        buffer.resize(buffer.size() + v.size());
        for (size_t i{0}; i < v.size(); ++i) {
            buffer[oldSize+i] = v[i];
        }
    };
    void writeVector(std::vector<int32_t> v) {
        if (v.size() < 15) { // No overflow
            auto l = (v.size() << 4) | 0x07;
            pack<uint8_t>(l);
        } else { // overflow
            pack<uint8_t>(0xf7);
            if (v.size() > 127) {
                throw "BCF: string to long";
            }
            writeInt<int8_t>(v.size());
        }
        for (auto e : v) {
            if ( e >= std::numeric_limits<int8_t>::lowest() and e <= std::numeric_limits<int8_t>::max()) {
                writeInt<int8_t>(e);
            } else if ( e >= -std::numeric_limits<int16_t>::lowest() and e <= std::numeric_limits<int16_t>::max()) {
                writeInt<int16_t>(e);
            } else {
                writeInt<int32_t>(e);
            }
        }
    };
    void writeData(std::span<uint8_t const> data) {
        auto oldSize = buffer.size();
        buffer.resize(oldSize + data.size());
        std::ranges::copy(data, begin(buffer) + oldSize);
    }

};

}



struct writer_pimpl {
    using Writers = std::variant<bgzf_file_writer/*,
                                 buffered_writer<zlib_file_writer>,
                                 stream_writer,
                                 buffered_writer<zlib_stream_writer>*/
                                 >;

    writer_config config;
    Writers writer;
    bcf_buffer buffer;

    writer_pimpl(writer_config config)
        : config{config}
        , writer {[&]() -> Writers {
            if (auto ptr = std::get_if<std::filesystem::path>(&config.output)) {
                return file_writer{ptr->c_str()};
            }
            throw std::runtime_error("unknown output type");
        }()}
    {}


};

writer::writer(writer_config config)
    : pimpl{std::make_unique<writer_pimpl>(config)}
{
}
writer::~writer() {
    if (pimpl) {
        std::visit([&](auto& writer) {
            writer.write({}, true);
        }, pimpl->writer);
    }
}

void writer::writeHeader(std::string_view v) {
    auto buffer = std::array<char, 9>{'B', 'C', 'F', 2, 2};

    bgzf_writer::detail::bgzfPack(static_cast<uint16_t>(v.size()), &buffer[5]);
    std::visit([&](auto& writer) {
       writer.write(buffer, false);
    }, pimpl->writer);
    std::visit([&](auto& writer) {
       writer.write(v, false);
    }, pimpl->writer);


}

void writer::write(record_view record) {
    assert(pimpl);

    auto const& [chrom, pos, id, ref, n_allele, alts, qual, filters, info, format, samples] = record;

    auto& buffer = pimpl->buffer;
    buffer.clear();


    auto l_shared = 0;
    auto l_indiv = 0;

    buffer.pack<uint32_t>(l_shared);
    buffer.pack<uint32_t>(l_indiv);

    auto chromId = contigMap.at(std::string{chrom}); //!TODO make it std::string_view compatible
    auto rlen = 0;
    buffer.pack<uint32_t>(chromId);
    buffer.pack<uint32_t>(pos-1);
    buffer.pack<uint32_t>(rlen);
    buffer.pack<float>(qual.value_or(0b0111'1111'1000'0000'0000'0000'0001));

    auto n_info = 0;
    auto n_sample = 0;
    auto n_fmt = 0;
    buffer.pack<int16_t>(n_info);
    buffer.pack<int16_t>(n_allele);
    buffer.pack<int32_t>(n_sample); // should actually just be 24bit
    bgzfPack<int8_t>(n_fmt, &buffer.buffer.back()); // overwrite last byte

    buffer.writeString(id);
    buffer.writeString(ref);

    buffer.writeData(alts);

    if (filters.empty()) {
        buffer.writeString("."); // symbol for missing
    } else {
        auto filterIds = std::vector<int32_t>{};
        for (auto const& f : filters) {
            filterMap.at(std::string{f}); //!TODO make it std::string_view compatible
        }
    }

    // copying the string into the buffer !TODO
    {
        auto oldSize = buffer.buffer.size();
        buffer.buffer.resize(buffer.buffer.size() + info.size());
        std::memcpy(buffer.buffer.data() + oldSize, info.data(), info.size());
    }

    // overwrite l_shared with actuall correct data
    l_shared = buffer.buffer.size() - 8;
    bgzfPack<uint32_t>(l_shared, buffer.buffer.data());

    //std::cout << "write: " << l_shared << " " << l_indiv << " " << chromId << " " << pos << " " << rlen << " " << qual.value_or(-1.f) << " " << n_info << " " << n_allele << " " << n_sample << " " << (int)n_fmt << "\n";

    //for (size_t i{0}; i < l_shared; ++i) {
    //    std::cout << (int) buffer.buffer[i+8] << ' ';
    //}
    //std::cout << '\n';



    std::visit([&](auto& writer) {
       writer.write(buffer.buffer, false);
    }, pimpl->writer);
}

}
