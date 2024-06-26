// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause
#include "../detail/bgzf_writer.h"
#include "writer.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <variant>

#include <iostream>

namespace {
template <typename T>
inline auto bgzfPack(T v, char* buffer) -> size_t {
    return ivio::bgzf_writer::detail::bgzfPack(v, buffer);
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
        ivio::bgzf_writer::detail::bgzfPack(v, buffer.data() + oldSize);
    }

    template <std::integral T>
    void writeInt(T v) {
        auto type = [&]() -> uint8_t {
            if constexpr (std::same_as<T, int8_t>) return 1;
            else if constexpr (std::same_as<T, int16_t>) return 2;
            else if constexpr (std::same_as<T, int32_t>) return 3;
            else throw std::runtime_error{"BCF error, expected an int(2)"};
        }();
        pack<uint8_t>(type);
        pack(v);
    }

    void writeString(std::string_view v) {
        if (v.size() < 15) { // No overflow
            auto l = (v.size() << 4) | 0x07;
            pack<uint8_t>(static_cast<uint8_t>(l));
        } else { // overflow
            pack<uint8_t>(0xf7);
            if (v.size() > 127) {
                throw std::runtime_error{"BCF: string to long"};
            }
            writeInt<int8_t>(static_cast<int8_t>(v.size()));
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
            pack<uint8_t>(static_cast<uint8_t>(l));
        } else { // overflow
            pack<uint8_t>(0xf7);
            if (v.size() > 127) {
                throw std::runtime_error{"BCF: string to long"};
            }
            writeInt<int8_t>(static_cast<int8_t>(v.size()));
        }
        for (auto e : v) {
            if (e >= std::numeric_limits<int8_t>::lowest() and e <= std::numeric_limits<int8_t>::max()) {
                writeInt<int8_t>(static_cast<int8_t>(e));
            } else if (e >= -std::numeric_limits<int16_t>::lowest() and e <= std::numeric_limits<int16_t>::max()) {
                writeInt<int16_t>(static_cast<int16_t>(e));
            } else {
                writeInt<int32_t>(e);
            }
        }
    };
    void writeData(std::span<uint8_t const> data) {
        auto oldSize = buffer.size();
        buffer.resize(oldSize + data.size());
//!WORKAROUND llvm < 16 does not provide std::ranges::copy
#if defined(_LIBCPP_VERSION) && _LIBCPP_VERSION < 160000
        std::copy(data.begin(), data.end(), begin(buffer) + oldSize);
#else
        std::ranges::copy(data, begin(buffer) + oldSize);
#endif
    }

};
}

template <>
struct ivio::writer_base<ivio::bcf::writer>::pimpl {
    //!TODO support other writers
    using Writers = std::variant<ivio::bgzf_file_writer>;


    Writers writer;
    bcf_buffer buffer;


    pimpl(std::filesystem::path output)
        : writer {[&]() -> Writers {
            return ivio::bgzf_file_writer{output};
        }()}
    {}

    pimpl(std::ostream& /*output*/)
        : writer {[&]() -> Writers {
            //!TODO
            throw std::runtime_error("streams are currently not supported");
        }()}
    {}
};


namespace ivio::bcf {

writer::writer(config config_)
    : writer_base{std::visit([&](auto& p) {
        return std::make_unique<pimpl>(p);
    }, config_.output)}
{
    // writing the header
    std::visit([&](auto& writer) {
        // write leading table
        auto ss = std::string{};
        ss = "##fileformat=VCFv4.3\n";

        for (auto [key, value] : config_.header.table) {
            if (key == "fileformat") continue; // ignore request to write file format
            ss += "##";
            ss += key;
            ss += '=';
            ss += value;
            ss += '\n';
        }
        // write heading of the body
        {
            ss += std::string{"#CHROM	POS	ID	REF	ALT	QUAL	FILTER	INFO	FORMAT"};
            for (auto const& s : config_.header.genotypes) {
                ss += '\t' + s;
            }
        }

        auto buffer = std::array<char, 9>{'B', 'C', 'F', 2, 2};
        bgzf_writer::detail::bgzfPack(static_cast<uint16_t>(ss.size()), &buffer[5]);
        writer.write(buffer);
        writer.write(ss);
    }, pimpl_->writer);
}

writer::~writer() = default;

void writer::write(record_view r) {
    assert(pimpl_);

    auto& buffer = pimpl_->buffer;
    buffer.clear();

    auto l_shared = 0;
    auto l_indiv = 0;

    buffer.pack<uint32_t>(l_shared);
    buffer.pack<uint32_t>(l_indiv);

    buffer.pack<int32_t>(r.chromId);
    buffer.pack<int32_t>(r.pos);
    buffer.pack<uint32_t>(r.rlen);
    buffer.pack<float>(r.qual.value_or(0b0111'1111'1000'0000'0000'0000'0001));

    buffer.pack<int16_t>(r.n_info);
    buffer.pack<int16_t>(r.n_allele);
    buffer.pack<int32_t>(r.n_sample); // should actually just be 24bit
    bgzfPack<int8_t>(r.n_fmt, &buffer.buffer.back()); // overwrite last byte

    buffer.writeString(r.id);
    buffer.writeString(r.ref);
    buffer.writeData(r.alt);
    buffer.writeData(r.filter);
    buffer.writeData(r.info);
    l_shared = static_cast<uint32_t>(buffer.buffer.size()) - 8;
    buffer.writeData(r.format);
    l_indiv = static_cast<uint32_t>(buffer.buffer.size()) - l_shared - 8;

    // overwrite l_shared with correct data
    bgzfPack<uint32_t>(l_shared, buffer.buffer.data());
    bgzfPack<uint32_t>(l_indiv,  buffer.buffer.data()+4);

    //std::cout << "write: " << l_shared << " " << l_indiv << " " << chromId << " " << pos << " " << rlen << " " << qual.value_or(-1.f) << " " << n_info << " " << n_allele << " " << n_sample << " " << (int)n_fmt << "\n";

    //for (size_t i{0}; i < l_shared; ++i) {
    //    std::cout << (int) buffer.buffer[i+8] << ' ';
    //}
    //std::cout << '\n';



    std::visit([&](auto& writer) {
       writer.write(buffer.buffer);
    }, pimpl_->writer);
}

void writer::close() {
    pimpl_.reset();
}

static_assert(record_writer_c<writer>);

}
