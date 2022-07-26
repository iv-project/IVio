#pragma once

#include "buffered_reader.h"
#include "file_reader.h"
#include "mmap_reader.h"
#include "stream_reader.h"

#include <zlib.h>
#include <ranges>

namespace io3 {

/*!\brief Convert the byte encoding of integer values to little-endian byte order.
 * \ingroup utility
 * \tparam type The type of the value to convert; must model std::integral.
 * \param  in   The input value to convert.
 * \returns the converted value in little-endian byte-order.
 *
 * \details
 *
 * This function swaps the bytes if the host system uses big endian. In this case only 1, 2, 4, or 8 byte big
 * integral types are allowed as input. On host systems with little endian this function is a no-op and returns the
 * unchanged input value. Other systems with mixed endianness are not supported.
 */
template <std::integral type>
constexpr type to_little_endian(type const in) noexcept
{
    if constexpr (std::endian::native == std::endian::little) {
        return in;
    }
    else if constexpr (std::endian::native == std::endian::big) {
        static_assert(sizeof(type) <= 8,
                      "Can only convert the byte encoding for integral numbers with a size of up to 8 bytes.");
        static_assert(std::has_single_bit(sizeof(type)),
                      "Can only convert the byte encoding for integral numbers whose byte size is a power of two.");

        if constexpr (sizeof(type) == 2)
            return htole16(in);
        else if constexpr (sizeof(type) == 4)
            return htole32(in);
        else if constexpr (sizeof(type) == 8)
            return htole64(in);
        else
            return in; // single byte.
    }
    else {
        static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big,
                      "Expected a little-endian or big-endian platform.");
    }
}


inline constexpr char _magic_bgzf_header[] =
    // GZip header
    "\x1f\x8b\x08"
    // FLG[MTIME         ] XFL OS [XLEN  ]
    "\x04\x00\x00\x00\x00\x00\xff\x06\x00"
    // B   C [SLEN  ][BSIZE ]
    "\x42\x43\x02\x00\x00\x00";

inline constexpr auto magic_bgzf_header = std::string_view{_magic_bgzf_header, sizeof(_magic_bgzf_header)};

template <typename T>
inline auto bgzfUnpack(char const* buffer) -> T {
    T v;
    std::uninitialized_copy(buffer, buffer + sizeof(v), reinterpret_cast<char*>(&v));
    return to_little_endian(v);
}

struct Context {
    z_stream strm{};
    int      compression_level{Z_DEFAULT_COMPRESSION};
    uint8_t  headerPos{};
    inline static constexpr size_t BlockHeaderLength = magic_bgzf_header.size();
    inline static constexpr size_t BlockFooterLength = 8;

    void decompressInit() {
        strm.zalloc = nullptr;
        strm.zfree  = nullptr;
        constexpr auto GzipWindowBits = -15; // no zlib header
        auto status = inflateInit2(&strm, GzipWindowBits);
        if (status != Z_OK) {
            throw "inflation failed";
        }
    }

    size_t decompressBlock(std::span<char const> in, std::span<char> out) {
        // 1. CHECK HEADER

        if (in.size() < BlockFooterLength)
            throw "BGZF block too short. " + std::to_string(in.size());

//        if (!detail::bgzf_compression::validate_header(std::span{srcBegin, srcLength}))
//            throw io_error("Invalid BGZF block header.");

        decompressInit();
        strm.next_in   = (Bytef *)in.data();
        strm.next_out  = (Bytef *)out.data();
        strm.avail_in  = in.size() - BlockFooterLength;
        strm.avail_out = out.size();

        auto status = inflate(&strm, Z_FINISH);
        if (status != Z_STREAM_END) {
            inflateEnd(&strm);
            throw "Inflation failed. Decompressed BGZF data is too big. " + std::to_string(strm.avail_in) + " " + std::to_string(strm.avail_out);
        }

        status = inflateEnd(&strm);
        if (status != Z_OK) {
            throw "BGZF inflateEnd() failed.";
        }
        // 3. CHECK FOOTER

        // Check compressed length in buffer, compute CRC and compare with CRC in buffer.
        unsigned crc = crc32(crc32(0, nullptr, 0), (Bytef *)out.data(), out.size() - strm.avail_out);
        unsigned ecrc = bgzfUnpack<uint32_t>(in.data() + in.size() - 8);
        if (ecrc != crc)
            throw "BGZF wrong checksum." + std::to_string(ecrc) + " " + std::to_string(crc);

        auto dlen = bgzfUnpack<uint32_t>(in.data() + in.size() - 4);
        if (dlen != out.size() - strm.avail_out)
            throw "BGZF size mismatch.";

        return out.size() - strm.avail_out;

    }
};


template <typename Reader>
struct bgzf_reader_impl {
    Reader file;

    Context ctx;

    template <typename T>
    bgzf_reader_impl(T&& name)
        : file(std::forward<T>(name))
    {}

    bgzf_reader_impl(bgzf_reader_impl&& _other)
        : file{std::move(_other.file)}
    {}

    ~bgzf_reader_impl() = default;

    size_t read(std::ranges::sized_range auto&& range) {
        while(true) {
            auto [ptr, avail_in] = file.read(18);
            if (avail_in == 0) return 0;
            if (avail_in < 18) throw "failed reading (1)";

            size_t compressedLen = bgzfUnpack<uint16_t>(ptr + 16) + 1u;
            auto [ptr2, avail_in2] = file.read(compressedLen);
            if (avail_in2 < compressedLen) throw "failed reading (2)";

            assert(range.size() >= (1<<16));

            size_t size = ctx.decompressBlock({ptr2+18, compressedLen-18}, {range.data(), range.size()});
            file.dropUntil(compressedLen);
            return size;
        }
    }
};

using bgzf_file_reader   = bgzf_reader_impl<buffered_reader<file_reader>>;
using bgzf_mmap_reader   = bgzf_reader_impl<mmap_reader>;
using bgzf_stream_reader = bgzf_reader_impl<buffered_reader<stream_reader>>;

static_assert(reader_c<bgzf_file_reader>);
static_assert(reader_c<bgzf_mmap_reader>);
static_assert(reader_c<bgzf_stream_reader>);

}
