/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

/*
References:
  - About Bitmaps (https://learn.microsoft.com/en-us/windows/win32/gdi/about-bitmaps)
  - BMP file format (https://en.wikipedia.org/wiki/BMP_file_format)
  - BMP Suite (https://entropymine.com/jason/bmpsuite)

=============================================================================

BMP Suite v2.7 decode status:

"bad":
b/badbitcount.bmp
b/badbitssize.bmp
b/baddens1.bmp           DECODED
b/baddens2.bmp           DECODED
b/badfilesize.bmp        DECODED
b/badheadersize.bmp
b/badpalettesize.bmp
b/badplanes.bmp
b/badrle.bmp
b/badrle4.bmp
b/badrle4bis.bmp
b/badrle4ter.bmp
b/badrlebis.bmp
b/badrleter.bmp
b/badwidth.bmp
b/pal8badindex.bmp
b/reallybig.bmp
b/rgb16-880.bmp          DECODED
b/rletopdown.bmp
b/shortfile.bmp

"good":
g/pal1.bmp               DECODED
g/pal1bg.bmp             DECODED
g/pal1wb.bmp             DECODED
g/pal4.bmp               DECODED
g/pal4gs.bmp             DECODED
g/pal4rle.bmp            DECODED
g/pal8.bmp               DECODED
g/pal8-0.bmp             DECODED
g/pal8gs.bmp             DECODED
g/pal8nonsquare.bmp      DECODED
g/pal8os2.bmp            DECODED
g/pal8rle.bmp            DECODED
g/pal8topdown.bmp        DECODED
g/pal8v4.bmp             DECODED
g/pal8v5.bmp             DECODED
g/pal8w124.bmp           DECODED
g/pal8w125.bmp           DECODED
g/pal8w126.bmp           DECODED
g/rgb16.bmp              DECODED
g/rgb16-565.bmp          DECODED
g/rgb16-565pal.bmp       DECODED
g/rgb16bfdef.bmp         DECODED
g/rgb24.bmp              DECODED
g/rgb24pal.bmp           DECODED
g/rgb32.bmp              DECODED
g/rgb32bf.bmp            DECODED
g/rgb32bfdef.bmp         DECODED

"questionable":
q/pal1hufflsb.bmp
q/pal1huffmsb.bmp
q/pal1p1.bmp             DECODED
q/pal2.bmp
q/pal2color.bmp
q/pal4rlecut.bmp         DECODED
q/pal4rletrns.bmp        DECODED
q/pal8offs.bmp           DECODED
q/pal8os2-hs.bmp         DECODED
q/pal8os2sp.bmp
q/pal8os2-sz.bmp         DECODED
q/pal8os2v2.bmp
q/pal8os2v2-16.bmp
q/pal8os2v2-40sz.bmp     DECODED
q/pal8os2v2-sz.bmp
q/pal8oversizepal.bmp    DECODED
q/pal8rlecut.bmp         DECODED
q/pal8rletrns.bmp        DECODED
q/rgb16-231.bmp          DECODED
q/rgb16-3103.bmp         DECODED
q/rgb16faketrns.bmp      DECODED
q/rgb24jpeg.bmp
q/rgb24largepal.bmp      DECODED
q/rgb24lprof.bmp         DECODED
q/rgb24png.bmp
q/rgb24prof.bmp          DECODED
q/rgb24prof2.bmp         DECODED
q/rgb24rle24.bmp
q/rgb32-111110.bmp       DECODED
q/rgb32-7187.bmp         DECODED
q/rgb32fakealpha.bmp     DECODED
q/rgb32h52.bmp
q/rgb32-xbgr.bmp         DECODED
q/rgba16-1924.bmp        DECODED
q/rgba16-4444.bmp        DECODED
q/rgba16-5551.bmp        DECODED
q/rgba32-1.bmp           DECODED
q/rgba32-1010102.bmp     DECODED
q/rgba32-2.bmp           DECODED
q/rgba32-61754.bmp       DECODED
q/rgba32-81284.bmp       DECODED
q/rgba32abf.bmp
q/rgba32h56.bmp
*/

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <memory>
#include <string>
#include "bstone_assert.h"
#include "bstone_bmp_image_common.h"
#include "bstone_bmp_image_decoder.h"
#include "bstone_endian.h"
#include "bstone_exception.h"

namespace bstone {

class BmpImageDecoderImpl
{
public:
	void decode(
		const void* src_data,
		std::intptr_t src_size,
		int& dst_width,
		int& dst_height,
		Rgba8Buffer& dst_bits);

private:
	static constexpr auto max_palette_size = std::intptr_t{256};

	using Palette = Rgba8[max_palette_size];

	struct Bitfield
	{
		bool is_alpha;
		int offset;
		int count;
		unsigned int mask;
		unsigned int max_value;
	};

	struct Bitfields
	{
		Bitfield r;
		Bitfield g;
		Bitfield b;
		Bitfield a;
	};

private:
	const unsigned char* src_bytes_{};
	const unsigned char* src_bytes_end_{};

	const unsigned char* remain_bytes_{};
	const unsigned char* remain_bytes_end_{};

	const unsigned char* bits_bytes_{};
	const unsigned char* bits_bytes_end_{};

	Rgba8* dst_bits_{};
	Rgba8* dst_bits_end_{};

	Rgba8* dst_rle_bits_{};
	Rgba8* dst_rle_line_{};
	Rgba8* dst_rle_line_end_{};

	bool is_vc_{};
	bool is_top_down_{};
	int width_{};
	int height_{};
	int bit_depth_{};
	int palette_size_{};
	unsigned int compression_{};
	Bitfields bfs_{};
	int stride_{};

	Palette palette_{};

private:
	// Returns the number of 1 bits in the value of x.
	static int popcount(unsigned int x) noexcept;

	static unsigned int get_high_nibble(unsigned int byte) noexcept;
	static unsigned int get_low_nibble(unsigned int byte) noexcept;

	// Returns a nibble by an index: 0 - high, 1 - low.
	static unsigned int get_nibble_by_index(unsigned int byte, unsigned int index) noexcept;

	static Bitfield make_bf_from_mask(unsigned int mask) noexcept;

	static unsigned int decode_bf(const Bitfield& bf, unsigned int value) noexcept;

	static Rgba8 bfs_to_a8b8g8r8(const Bitfields& bfs, unsigned int value) noexcept;

	static Rgba8 x1r5g5b5_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept;
	static Rgba8 a1r5g5b5_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept;
	static Rgba8 x0r5g6b5_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept;

	static Rgba8 x8r8g8b8_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept;
	static Rgba8 a8r8g8b8_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept;

private:
	template<typename T>
	T generic_read_le();

	void skip_bytes(std::intptr_t count);

	template<typename T>
	void skip_generic();

	int read_u8();
	int read_u16_le();
	int read_s32_le();
	unsigned int read_u32_le();

	void skip_u16();
	void skip_u32();

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	// Routines "read_?bpp_u32" does not check for source data underflow.
	// We already validated it earlier.

	unsigned int read_1bpp_u32();
	unsigned int read_4bpp_u32();
	unsigned int read_8bpp_u32();
	unsigned int read_16bpp_u32();
	unsigned int read_24bpp_u32();
	unsigned int read_32bpp_u32();

	// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	void skip_line_padding(std::intptr_t size);

	void rle_write_pixel_by_index(std::intptr_t index);
	void rle_write_pixel(Rgba8 color);
	void rle_end_of_line();
	void rle_move_cursor();

	void import_bitmap_file_header();
	void import_bitmap_vx_header();

	void import_bgr_palette();
	void import_bgrx_palette();
	void import_palette();

	void decode_1bpp();

	void decode_4bpp_rle();
	void decode_4bpp_uncompressed();
	void decode_4bpp();

	void decode_8bpp_rle();
	void decode_8bpp_uncompressed();
	void decode_8bpp();

	void decode_16bpp();

	void decode_24bpp();

	void decode_32bpp();

	void decode_internal();
};

// ==========================================================================

void BmpImageDecoderImpl::decode(
	const void* src_data,
	std::intptr_t src_size,
	int& dst_width,
	int& dst_height,
	Rgba8Buffer& dst_bits)
try {
	dst_width = 0;
	dst_height = 0;

	if (src_data == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null source data.");
	}

	if (src_size < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Source data size out of range.");
	}

	src_bytes_ = static_cast<const unsigned char*>(src_data);
	src_bytes_end_ = src_bytes_ + src_size;

	remain_bytes_ = src_bytes_;
	remain_bytes_end_ = src_bytes_end_;

	import_bitmap_file_header();
	import_bitmap_vx_header();
	import_palette();

	const auto area = width_ * height_;
	dst_bits.clear();
	dst_bits.resize(area);

	dst_bits_ = dst_bits.data();
	dst_bits_end_ = dst_bits_ + area;

	decode_internal();

	dst_width = width_;
	dst_height = height_;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// Returns the number of 1 bits in the value of x.
int BmpImageDecoderImpl::popcount(unsigned int x) noexcept
{
	auto count = 0;

	while (x != 0U)
	{
		if ((x & 1U) != 0U)
		{
			++count;
		}

		x >>= 1;
	}

	return count;
}

unsigned int BmpImageDecoderImpl::get_high_nibble(unsigned int byte) noexcept
{
	return (byte >> 4) & 0xFU;
}

unsigned int BmpImageDecoderImpl::get_low_nibble(unsigned int byte) noexcept
{
	return byte & 0xFU;
}

// 0 - high nibble, 1 - low nibble.
unsigned int BmpImageDecoderImpl::get_nibble_by_index(unsigned int byte, unsigned int index) noexcept
{
	return (byte >> (4 * (1 - index))) & 0xFU;
}

auto BmpImageDecoderImpl::make_bf_from_mask(unsigned int mask) noexcept -> Bitfield
{
	auto offset = 0;
	auto bit = 1U;

	for (auto i = 0; i < 32; ++i)
	{
		if ((mask & bit) != 0U)
		{
			break;
		}

		bit <<= 1;
		++offset;
	}

	auto count = 0;

	while ((mask & bit) != 0U)
	{
		bit <<= 1;
		++count;
	}

	auto bf = Bitfield{};
	bf.offset = (offset < 32 ? offset : 0);
	bf.count = count;
	bf.mask = mask;
	bf.max_value = (~0U) >> (32 - count);
	return bf;
}

unsigned int BmpImageDecoderImpl::decode_bf(const Bitfield& bf, unsigned int value) noexcept
{
	if (bf.count == 0)
	{
		return bf.is_alpha ? 0xFFU : 0U;
	}

	return (((value & bf.mask) >> bf.offset) * 255U) / bf.max_value;
}

Rgba8 BmpImageDecoderImpl::bfs_to_a8b8g8r8(const Bitfields& bfs, unsigned int value) noexcept
{
	auto result = Rgba8{};
	result.r_ = static_cast<std::uint8_t>(decode_bf(bfs.r, value));
	result.g_ = static_cast<std::uint8_t>(decode_bf(bfs.g, value));
	result.b_ = static_cast<std::uint8_t>(decode_bf(bfs.b, value));
	result.a_ = static_cast<std::uint8_t>(decode_bf(bfs.a, value));
	return result;
}

Rgba8 BmpImageDecoderImpl::x1r5g5b5_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept
{
	auto result = Rgba8{};
	result.r_ = static_cast<std::uint8_t>((((value >> 10) & 0x1FU) * 255U) / 31U);
	result.g_ = static_cast<std::uint8_t>((((value >> 5) & 0x1FU) * 255U) / 31U);
	result.b_ = static_cast<std::uint8_t>(((value & 0x1FU) * 255U) / 31U);
	result.a_ = 0xFFU;
	return result;
}

Rgba8 BmpImageDecoderImpl::a1r5g5b5_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept
{
	auto result = Rgba8{};
	result.r_ = static_cast<std::uint8_t>((((value >> 10) & 0x1FU) * 255U) / 31U);
	result.g_ = static_cast<std::uint8_t>((((value >> 5) & 0x1FU) * 255U) / 31U);
	result.b_ = static_cast<std::uint8_t>(((value & 0x1FU) * 255U) / 31U);
	result.a_ = static_cast<std::uint8_t>(((value & 0x8000U) != 0U ? 0xFFU : 0U));
	return result;
}

Rgba8 BmpImageDecoderImpl::x0r5g6b5_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept
{
	auto result = Rgba8{};
	result.r_ = static_cast<std::uint8_t>((((value >> 11) & 0x1FU) * 255U) / 31U);
	result.g_ = static_cast<std::uint8_t>((((value >> 5) & 0x3FU) * 255U) / 63U);
	result.b_ = static_cast<std::uint8_t>(((value & 0x1FU) * 255U) / 31U);
	result.a_ = 0xFFU;
	return result;
}

Rgba8 BmpImageDecoderImpl::x8r8g8b8_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept
{
	auto result = Rgba8{};
	result.r_ = static_cast<std::uint8_t>((value >> 16) & 0xFFU);
	result.g_ = static_cast<std::uint8_t>((value >> 8) & 0xFFU);
	result.b_ = static_cast<std::uint8_t>(value & 0xFFU);
	result.a_ = 0xFFU;
	return result;
}

Rgba8 BmpImageDecoderImpl::a8r8g8b8_to_a8b8g8r8(const Bitfields&, unsigned int value) noexcept
{
	auto result = Rgba8{};
	result.r_ = static_cast<std::uint8_t>((value >> 16) & 0xFFU);
	result.g_ = static_cast<std::uint8_t>((value >> 8) & 0xFFU);
	result.b_ = static_cast<std::uint8_t>(value & 0xFFU);
	result.a_ = static_cast<std::uint8_t>((value >> 24) & 0xFFU);
	return result;
}

template<typename T>
T BmpImageDecoderImpl::generic_read_le()
{
	const auto type_size = static_cast<std::intptr_t>(sizeof(T));

	if ((remain_bytes_end_ - remain_bytes_) < type_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Data underflow.");
	}

	const auto result = endian::to_little(*reinterpret_cast<const T*>(remain_bytes_));
	remain_bytes_ += type_size;
	return result;
}

void BmpImageDecoderImpl::skip_bytes(std::intptr_t count)
{
	BSTONE_ASSERT(count >= 0);
	remain_bytes_ += count;
}

template<typename T>
void BmpImageDecoderImpl::skip_generic()
{
	constexpr auto type_size = static_cast<std::intptr_t>(sizeof(T));
	skip_bytes(type_size);
}

int BmpImageDecoderImpl::read_u8()
{
	return generic_read_le<std::uint8_t>();
}

int BmpImageDecoderImpl::read_u16_le()
{
	return generic_read_le<std::uint16_t>();
}

int BmpImageDecoderImpl::read_s32_le()
{
	return generic_read_le<std::int32_t>();
}

unsigned int BmpImageDecoderImpl::read_u32_le()
{
	return generic_read_le<std::uint32_t>();
}

void BmpImageDecoderImpl::skip_u16()
{
	skip_generic<std::uint16_t>();
}

void BmpImageDecoderImpl::skip_u32()
{
	skip_generic<std::uint32_t>();
}

unsigned int BmpImageDecoderImpl::read_1bpp_u32()
{
	return *remain_bytes_++;
}

unsigned int BmpImageDecoderImpl::read_4bpp_u32()
{
	return read_1bpp_u32();
}

unsigned int BmpImageDecoderImpl::read_8bpp_u32()
{
	return read_1bpp_u32();
}

unsigned int BmpImageDecoderImpl::read_16bpp_u32()
{
	const auto result = endian::to_little(*reinterpret_cast<const std::uint16_t*>(remain_bytes_));
	remain_bytes_ += 2;
	return result;
}

unsigned int BmpImageDecoderImpl::read_24bpp_u32()
{
	const auto result = endian::to_little(*reinterpret_cast<const std::uint32_t*>(remain_bytes_));
	remain_bytes_ += 3;
	return result;
}

unsigned int BmpImageDecoderImpl::read_32bpp_u32()
{
	const auto result = endian::to_little(*reinterpret_cast<const std::uint32_t*>(remain_bytes_));
	remain_bytes_ += 4;
	return result;
}

void BmpImageDecoderImpl::rle_write_pixel(Rgba8 color)
{
	*dst_rle_line_++ = color;
}

void BmpImageDecoderImpl::skip_line_padding(std::intptr_t size)
{
	BSTONE_ASSERT(size >= 0);
	remain_bytes_ += size;
}

void BmpImageDecoderImpl::rle_write_pixel_by_index(std::intptr_t index)
{
	if (dst_rle_line_ == dst_rle_line_end_)
	{
		BSTONE_THROW_STATIC_SOURCE("RLE scan-line write overflow.");
	}

	rle_write_pixel(palette_[index]);
}

void BmpImageDecoderImpl::rle_end_of_line()
{
	dst_rle_bits_ -= width_;

	if (dst_rle_bits_ < dst_bits_)
	{
		BSTONE_THROW_STATIC_SOURCE("RLE-EOL Effective position out of range.");
	}

	dst_rle_line_ = dst_rle_bits_;
	dst_rle_line_end_ = dst_rle_line_ + width_;
}

void BmpImageDecoderImpl::rle_move_cursor()
{
	const auto dx = read_u8();
	const auto remain_width = dst_rle_line_end_ - dst_rle_line_;

	if (dx > remain_width)
	{
		BSTONE_THROW_STATIC_SOURCE("RLE-DELTA X overflow.");
	}

	const auto dy = read_u8();
	const auto skip_full_size = width_ * dy;
	dst_rle_bits_ -= skip_full_size;

	if (dst_rle_bits_ < dst_bits_)
	{
		BSTONE_THROW_STATIC_SOURCE("RLE-DELTA Y overflow.");
	}

	dst_rle_bits_ -= skip_full_size;
	dst_rle_line_ += dx;
	dst_rle_line_end_ = dst_rle_bits_ + width_;
}

// Imports BITMAPFILEHEADER.
void BmpImageDecoderImpl::import_bitmap_file_header()
{
	// bfType
	//
	const auto type = read_u16_le();

	if (type != bmp::type_bm)
	{
		BSTONE_THROW_STATIC_SOURCE("Unknown image format.");
	}

	// bfSize
	//
	skip_u32();

	// bfReserved1
	//
	skip_u16();

	// bfReserved2
	//
	skip_u16();

	// bfOffBits
	//
	const auto off_bits = read_u32_le();

	if ((src_bytes_ + off_bits) > src_bytes_end_)
	{
		BSTONE_THROW_STATIC_SOURCE("Image bits offset out of range.");
	}

	// Store required properties.
	//
	bits_bytes_ = src_bytes_ + off_bits;
}

// Imports BITMAPCOREHEADER, BITMAPINFOHEADER, BITMAPV4HEADER or BITMAPV5HEADER.
void BmpImageDecoderImpl::import_bitmap_vx_header()
{
	// biSize
	//
	const auto size = read_u32_le();

	const auto is_vc = (size == bmp::bitmapcoreheader_size);
	const auto is_vi = (size == bmp::bitmapinfoheader_size);
	const auto is_v4 = (size == bmp::bitmapv4header_size);
	const auto is_v5 = (size == bmp::bitmapv5header_size);

	if (!is_vc && !is_vi && !is_v4 && !is_v5)
	{
		BSTONE_THROW_STATIC_SOURCE("Unknown version.");
	}

	// biWidth
	//
	const auto width = (is_vc ? read_u16_le() : read_s32_le());

	if (width <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Width out of range.");
	}

	// biHeight
	//
	const auto height = (is_vc ? read_u16_le() : read_s32_le());

	if (height == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Height out of range.");
	}

	// biPlanes
	//
	const auto planes = read_u16_le();

	if (planes != bmp::plane_count)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid plane count.");
	}

	// biBitCount
	//
	const auto bit_count = read_u16_le();

	if (is_vc)
	{
		switch (bit_count)
		{
			case 1:
			case 4:
			case 8:
			case 24:
				break;

			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown bit depth.");
		}
	}
	else
	{
		switch (bit_count)
		{
			case 1:
			case 4:
			case 8:
			case 16:
			case 24:
			case 32:
				break;

			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown bit depth.");
		}
	}

	auto compression = bmp::bi_rgb;
	auto size_image = std::uint32_t{};
	auto clr_used = std::uint32_t{};
	auto clr_important = std::uint32_t{};

	if (is_vi || is_v4 || is_v5)
	{
		// biCompression
		//
		compression = read_u32_le();

		if (compression != bmp::bi_rgb &&
			compression != bmp::bi_rle4 &&
			compression != bmp::bi_rle8 &&
			compression != bmp::bi_bitfields)
		{
			BSTONE_THROW_STATIC_SOURCE("Unknown compression type.");
		}

		// biSizeImage
		//
		size_image = read_u32_le();

		if (size_image > bmp::max_int32)
		{
			BSTONE_THROW_STATIC_SOURCE("Total size of image bits out of range.");
		}

		if (size_image == 0 && compression != bmp::bi_rgb && compression != bmp::bi_bitfields)
		{
			BSTONE_THROW_STATIC_SOURCE("Expected total size of image bits.");
		}

		// biXPelsPerMeter
		//
		const auto x_pels_per_meter = read_s32_le();

		if (x_pels_per_meter < 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Horizontal resolution of the traget device out of range.");
		}

		// biYPelsPerMeter
		//
		const auto y_pels_per_meter = read_s32_le();

		if (y_pels_per_meter < 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Vertical resolution of the traget device out of range.");
		}

		// biClrUsed
		//
		clr_used = read_u32_le();

		if (clr_used > bmp::max_int32)
		{
			BSTONE_THROW_STATIC_SOURCE("Palette size ouf of range.");
		}

		// biClrImportant
		//
		clr_important = read_u32_le();

		if (clr_important > bmp::max_int32)
		{
			BSTONE_THROW_STATIC_SOURCE("Important color count ouf of range.");
		}
	}

	// ----------------------------------------------------------------------
	// V4-specific and shared V5-specific properties

	auto red_mask = std::uint32_t{};
	auto green_mask = std::uint32_t{};
	auto blue_mask = std::uint32_t{};
	auto alpha_mask = std::uint32_t{};

	if (is_v4 || is_v5)
	{
		// Color masks.
		//
		red_mask = read_u32_le(); // bV4RedMask / bV5RedMask
		green_mask = read_u32_le(); // bV4GreenMask / bV5GreenMask
		blue_mask = read_u32_le(); // bV4BlueMask / bV5BlueMask
		alpha_mask = read_u32_le(); // bV4AlphaMask / bV5AlphaMask

		// bV4CSType / bV5CSType
		//
		const auto cs_type = read_u32_le();

		switch (cs_type)
		{
			case bmp::lcs_calibrated_rgb:
			case bmp::lcs_srgb:
			case bmp::lcs_windows_color_space:
			case bmp::profile_linked:
			case bmp::profile_embedded:
				break;

			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown v5 color space.");
		}

		// bV4Endpoints / bV5Endpoints
		//
		// TODO
		skip_bytes(bmp::ciexyztriple_size);

		// bV4GammaRed / bV5GammaRed
		//
		// TODO
		skip_u32();

		// bV4GammaGreen / bV5GammaGreen
		//
		// TODO
		skip_u32();

		// bV4GammaBlue / bV5GammaBlue
		//
		// TODO
		skip_u32();
	}

	// ----------------------------------------------------------------------
	// V5-specific properties

	if (is_v5)
	{
		// bV5Intent
		//
		// TODO
		const auto intent = read_u32_le();

		switch (intent)
		{
			case bmp::lcs_gm_business:
			case bmp::lcs_gm_graphics:
			case bmp::lcs_gm_images:
			case bmp::lcs_gm_abs_colorimetric:
				break;

			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown v5 rendering intent.");
		}

		// bV5ProfileData
		//
		// TODO
		skip_u32();


		// bV5ProfileSize
		//
		// TODO
		skip_u32();

		// bV5Reserved
		//
		skip_u32();
	}

	// ----------------------------------------------------------------------
	// Optional color masks.

	auto is_read_masks = false;

	if (!is_vc && !is_v4 && !is_v5 && compression == bmp::bi_bitfields)
	{
		is_read_masks = true;

		red_mask = read_u32_le();
		green_mask = read_u32_le();
		blue_mask = read_u32_le();
	}

	// ----------------------------------------------------------------------
	// Adjust palette size.

	if (clr_used == 0U && bit_count < 16)
	{
		clr_used = 1U << bit_count;
	}

	if (clr_important == 0U)
	{
		clr_important = clr_used;
	}

	// ----------------------------------------------------------------------
	// Additional validation.

	const auto max_height = ((bmp::max_int32 / width) / 4) * 4;

	if (std::abs(height) > max_height)
	{
		BSTONE_THROW_STATIC_SOURCE("Image area out of range.");
	}

	if (height < 0 && (compression == bmp::bi_rle4 || compression == bmp::bi_rle8))
	{
		BSTONE_THROW_STATIC_SOURCE("Top-down compressed image.");
	}

	if (compression == bmp::bi_rle4 && bit_count != 4)
	{
		BSTONE_THROW_STATIC_SOURCE("RLE4 Expected 4-bit depth.");
	}

	if (compression == bmp::bi_rle8 && bit_count != 8)
	{
		BSTONE_THROW_STATIC_SOURCE("RLE8 Expected 8-bit depth.");
	}

	if (clr_used > 0U && clr_important > clr_used)
	{
		BSTONE_THROW_STATIC_SOURCE("Too many important colors.");
	}

	if (clr_used > 0U)
	{
		const auto palette_data_size = ((((is_vc ? 3U : 4U) * clr_used) + 3U) / 4U) * 4U;

		const auto min_bits_offset =
			bmp::bitmapfileheader_size +
			size +
			(is_read_masks ? (3U * 4U) : 0U) +
			palette_data_size;

		if (bits_bytes_ < (src_bytes_ + min_bits_offset))
		{
			BSTONE_THROW_STATIC_SOURCE("Palette overlaps with image bits.");
		}
	}

	if (compression == bmp::bi_bitfields)
	{
		if (bit_count != 16 && bit_count != 32)
		{
			BSTONE_THROW_STATIC_SOURCE("BITFIELDS Invalid bit depth.");
		}

		const auto red_mask_ones_count = popcount(red_mask);
		const auto green_mask_ones_count = popcount(green_mask);
		const auto blue_mask_ones_count = popcount(blue_mask);
		const auto alpha_mask_ones_count = popcount(alpha_mask);

		const auto rgba_ones_count =
			red_mask_ones_count +
			green_mask_ones_count +
			blue_mask_ones_count +
			alpha_mask_ones_count;

		const auto all_mask = red_mask | green_mask | blue_mask | alpha_mask;
		const auto all_mask_ones_count = popcount(all_mask);

		if (all_mask_ones_count != rgba_ones_count)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid mask combination.");
		}

		if (rgba_ones_count > bit_count)
		{
			BSTONE_THROW_STATIC_SOURCE("Mask bit count overflow.");
		}
	}

	const auto stride = bmp::calculate_stride(width, bit_count);

	if (size_image == 0U)
	{
		if ((bmp::max_int32 / stride) < height)
		{
			BSTONE_THROW_STATIC_SOURCE("Image dimensions too big.");
		}

		size_image = stride * height;
	}

	if ((src_bytes_end_ - bits_bytes_) < size_image)
	{
		BSTONE_THROW_STATIC_SOURCE("Total size of image bits out of range.");
	}

	if (bits_bytes_end_ > src_bytes_end_)
	{
		BSTONE_THROW_STATIC_SOURCE("Image bits are outside of the source data.");
	}

	// ----------------------------------------------------------------------
	// Store required properties.

	bits_bytes_end_ = bits_bytes_ + size_image;

	is_vc_ = is_vc;
	is_top_down_ = (height < 0);
	width_ = width;
	height_ = (is_top_down_ ? -height : height);
	bit_depth_ = bit_count;
	palette_size_ = static_cast<int>(clr_used);
	compression_ = compression;

	bfs_.r = make_bf_from_mask(red_mask);
	bfs_.g = make_bf_from_mask(green_mask);
	bfs_.b = make_bf_from_mask(blue_mask);
	bfs_.a = make_bf_from_mask(alpha_mask);
	bfs_.a.is_alpha = true;

	stride_ = stride;

	if (bits_bytes_ == bits_bytes_end_ && compression_ == bmp::bi_rgb)
	{
		bits_bytes_end_ = bits_bytes_ + (stride_ * height_);
	}

	if (palette_size_ > max_palette_size)
	{
		palette_size_ = max_palette_size;
	}
}

void BmpImageDecoderImpl::import_bgr_palette()
{
	const auto data_size = 3 * palette_size_;

	if ((remain_bytes_end_ - remain_bytes_) < data_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Unexpected end of BGR palette.");
	}

	struct Bgr
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
	};

	const auto src_colors = reinterpret_cast<const Bgr*>(remain_bytes_);

	for (auto i = 0; i < palette_size_; ++i)
	{
		const auto& src_color = src_colors[i];
		auto& dst_color = palette_[i];
		dst_color.r_ = src_color.r;
		dst_color.g_ = src_color.g;
		dst_color.b_ = src_color.b;
		dst_color.a_ = 0xFFU;
	}

	skip_bytes(data_size);
}

void BmpImageDecoderImpl::import_bgrx_palette()
{
	for (auto i = 0; i < palette_size_; ++i)
	{
		const auto src_color = read_u32_le();
		auto& dst_color = palette_[i];
		dst_color.r_ = static_cast<std::uint8_t>((src_color >> 16) & 0xFFU);
		dst_color.g_ = static_cast<std::uint8_t>((src_color >> 8) & 0xFFU);
		dst_color.b_ = static_cast<std::uint8_t>(src_color & 0xFFU);
		dst_color.a_ = 0xFFU;
	}
}

void BmpImageDecoderImpl::import_palette()
{
	if (bit_depth_ >= 16)
	{
		return;
	}

	if (is_vc_)
	{
		import_bgr_palette();
	}
	else
	{
		import_bgrx_palette();
	}
}

void BmpImageDecoderImpl::decode_1bpp()
{
	const auto line_size = (width_ + 7) / 8;
	const auto padding_size = stride_ - line_size;

	auto dst_bits = dst_bits_ + (is_top_down_ ? 0 : width_ * (height_ - 1));
	const auto dst_line_step = (is_top_down_ ? 0 : (-2 * width_));

	for (auto h = decltype(height_){}; h < height_; ++h)
	{
		auto bit_mask = 0U;
		auto byte_cache = 0U;

		for (auto w = decltype(width_){}; w < width_; ++w)
		{
			if (bit_mask == 0U)
			{
				bit_mask = 0x80U;
				byte_cache = read_1bpp_u32();
			}

			const auto index = ((byte_cache & bit_mask) == 0U ? 0 : 1);

			if (index >= palette_size_)
			{
				BSTONE_THROW_STATIC_SOURCE("1-bpp color index out of range.");
			}

			*dst_bits++ = palette_[index];
			bit_mask >>= 1;
		}

		skip_line_padding(padding_size);
		dst_bits += dst_line_step;
	}
}

void BmpImageDecoderImpl::decode_4bpp_rle()
{
	dst_rle_bits_ = dst_bits_ + (width_ * (height_ - 1));
	dst_rle_line_ = dst_rle_bits_;
	dst_rle_line_end_ = dst_rle_line_ + width_;

	while (true)
	{
		const auto byte_1 = read_u8();

		if (byte_1 == 0)
		{
			const auto byte_2 = read_u8();

			if (byte_2 == 0)
			{
				// End of line.
				//
				rle_end_of_line();
			}
			else if (byte_2 == 1)
			{
				// End of bitmap.
				//
				break;
			}
			else if (byte_2 == 2)
			{
				// Delta.
				//
				rle_move_cursor();
			}
			else
			{
				// Absolute mode.
				//
				const auto count = byte_2;
				auto nibble_index = 0U;
				auto byte_cache = 0U;

				for (auto i = 0; i < count; ++i)
				{
					if (nibble_index == 0U)
					{
						byte_cache = static_cast<unsigned int>(read_u8());
					}

					const auto index = get_nibble_by_index(byte_cache, nibble_index);
					nibble_index ^= 1U;

					rle_write_pixel_by_index(static_cast<std::intptr_t>(index));
				}

				// Skip a padding byte.
				const auto byte_count = (count + 1) / 2;
				skip_bytes(byte_count % 2);
			}
		}
		else
		{
			// Encoded mode.
			//
			const auto count = byte_1;
			const auto byte_2 = read_u8();
			const auto color_index_0 = get_high_nibble(byte_2);
			const auto color_index_1 = get_low_nibble(byte_2);

			if (static_cast<int>(color_index_0) >= palette_size_ ||
				static_cast<int>(color_index_1) >= palette_size_)
			{
				BSTONE_THROW_STATIC_SOURCE("RLE-ENC Color index out of range.");
			}

			const Rgba8 color_cache[2] = {palette_[color_index_0], palette_[color_index_1]};

			auto cache_index = 0U;

			for (auto i = 0; i < count; ++i)
			{
				rle_write_pixel(color_cache[cache_index]);
				cache_index ^= 1U;
			}
		}
	}
}

void BmpImageDecoderImpl::decode_4bpp_uncompressed()
{
	const auto line_size = ((4 * width_) + 7) / 8;
	const auto padding_size = stride_ - line_size;

	auto dst_bits = dst_bits_ + (is_top_down_ ? 0 : width_ * (height_ - 1));
	const auto dst_line_step = (is_top_down_ ? 0 : (-2 * width_));

	for (auto h = decltype(height_){}; h < height_; ++h)
	{
		auto nibble_index = 0U;
		auto byte_cache = 0U;

		for (auto w = decltype(width_){}; w < width_; ++w)
		{
			if (nibble_index == 0)
			{
				byte_cache = read_4bpp_u32();
			}

			const auto index = get_nibble_by_index(byte_cache, nibble_index);
			nibble_index ^= 1U;

			if (static_cast<int>(index) >= palette_size_)
			{
				BSTONE_THROW_STATIC_SOURCE("4-bpp color index out of range.");
			}

			*dst_bits++ = palette_[index];
		}

		skip_line_padding(padding_size);
		dst_bits += dst_line_step;
	}
}

void BmpImageDecoderImpl::decode_4bpp()
{
	if (compression_ == bmp::bi_rle4)
	{
		decode_4bpp_rle();
	}
	else
	{
		decode_4bpp_uncompressed();
	}
}

void BmpImageDecoderImpl::decode_8bpp_rle()
{
	dst_rle_bits_ = dst_bits_ + (width_ * (height_ - 1));
	dst_rle_line_ = dst_rle_bits_;
	dst_rle_line_end_ = dst_rle_line_ + width_;

	while (true)
	{
		const auto byte_1 = read_u8();

		if (byte_1 == 0)
		{
			const auto byte_2 = read_u8();

			if (byte_2 == 0)
			{
				// End of line.
				//
				rle_end_of_line();
			}
			else if (byte_2 == 1)
			{
				// End of bitmap.
				//
				break;
			}
			else if (byte_2 == 2)
			{
				// Delta.
				//
				rle_move_cursor();
			}
			else
			{
				// Absolute mode.
				//
				const auto count = byte_2;

				for (auto i = 0; i < count; ++i)
				{
					const auto index = read_u8();
					rle_write_pixel_by_index(index);
				}

				// Skip a padding byte.
				skip_bytes(count % 2);
			}
		}
		else
		{
			// Encoded mode.
			//
			const auto count = byte_1;
			const auto index = read_u8();

			if (index >= palette_size_)
			{
				BSTONE_THROW_STATIC_SOURCE("RLE-ENC Color index out of range.");
			}

			const auto color = palette_[index];

			for (auto i = 0; i < count; ++i)
			{
				rle_write_pixel(color);
			}
		}
	}
}

void BmpImageDecoderImpl::decode_8bpp_uncompressed()
{
	const auto padding_size = stride_ - width_;

	auto dst_bits = dst_bits_ + (is_top_down_ ? 0 : width_ * (height_ - 1));
	const auto dst_line_step = (is_top_down_ ? 0 : (-2 * width_));

	for (auto h = decltype(height_){}; h < height_; ++h)
	{
		for (auto w = decltype(width_){}; w < width_; ++w)
		{
			const auto index = read_8bpp_u32();

			if (static_cast<int>(index) >= palette_size_)
			{
				BSTONE_THROW_STATIC_SOURCE("8-bpp color index out of range.");
			}

			*dst_bits++ = palette_[index];
		}

		skip_line_padding(padding_size);
		dst_bits += dst_line_step;
	}
}

void BmpImageDecoderImpl::decode_8bpp()
{
	if (compression_ == bmp::bi_rle8)
	{
		decode_8bpp_rle();
	}
	else
	{
		decode_8bpp_uncompressed();
	}
}

void BmpImageDecoderImpl::decode_16bpp()
{
	auto bf_func = bfs_to_a8b8g8r8;

	if (compression_ == bmp::bi_rgb)
	{
		bf_func = x1r5g5b5_to_a8b8g8r8;
	}
	else
	{
		const auto& r_bf = bfs_.r;
		const auto& g_bf = bfs_.g;
		const auto& b_bf = bfs_.b;
		const auto& a_bf = bfs_.a;

		if (false)
		{}
		else if (
			r_bf.offset == 10 && r_bf.count == 5 &&
			g_bf.offset == 5 && g_bf.count == 5 &&
			b_bf.offset == 0 && b_bf.count == 5 &&
			a_bf.count == 0)
		{
			bf_func = x1r5g5b5_to_a8b8g8r8;
		}
		else if (
			r_bf.offset == 10 && r_bf.count == 5 &&
			g_bf.offset == 5 && g_bf.count == 5 &&
			b_bf.offset == 0 && b_bf.count == 5 &&
			a_bf.count == 1 && a_bf.offset == 15)
		{
			bf_func = a1r5g5b5_to_a8b8g8r8;
		}
		else if (
			r_bf.offset == 11 && r_bf.count == 5 &&
			g_bf.offset == 5 && g_bf.count == 6 &&
			b_bf.offset == 0 && b_bf.count == 5 &&
			a_bf.count == 0)
		{
			bf_func = x0r5g6b5_to_a8b8g8r8;
		}
	}

	const auto line_size = 2 * width_;
	const auto padding_size = stride_ - line_size;

	auto dst_bits = dst_bits_ + (is_top_down_ ? 0 : width_ * (height_ - 1));
	const auto dst_line_step = (is_top_down_ ? 0 : (-2 * width_));

	for (auto h = decltype(height_){}; h < height_; ++h)
	{
		for (auto w = decltype(width_){}; w < width_; ++w)
		{
			const auto src_pixel = read_16bpp_u32();
			*dst_bits++ = bf_func(bfs_, src_pixel);
		}

		skip_line_padding(padding_size);
		dst_bits += dst_line_step;
	}
}

void BmpImageDecoderImpl::decode_24bpp()
{
	const auto line_size = 3 * width_;
	const auto padding_size = stride_ - line_size;

	auto dst_bits = dst_bits_ + (is_top_down_ ? 0 : width_ * (height_ - 1));
	const auto dst_line_step = (is_top_down_ ? 0 : (-2 * width_));

	for (auto h = decltype(height_){}; h < height_; ++h)
	{
		for (auto w = decltype(width_){}; w < width_; ++w)
		{
			const auto src_color = read_24bpp_u32();
			auto dst_color = Rgba8{};
			dst_color.r_ = static_cast<std::uint8_t>((src_color >> 16) & 0xFFU);
			dst_color.g_ = static_cast<std::uint8_t>((src_color >> 8) & 0xFFU);
			dst_color.b_ = static_cast<std::uint8_t>(src_color & 0xFFU);
			dst_color.a_ = 0xFFU;
			*dst_bits++ = dst_color;
		}

		skip_line_padding(padding_size);
		dst_bits += dst_line_step;
	}
}

void BmpImageDecoderImpl::decode_32bpp()
{
	auto bf_func = bfs_to_a8b8g8r8;

	if (compression_ == bmp::bi_rgb)
	{
		bf_func = x8r8g8b8_to_a8b8g8r8;
	}
	else
	{
		const auto& r_bf = bfs_.r;
		const auto& g_bf = bfs_.g;
		const auto& b_bf = bfs_.b;
		const auto& a_bf = bfs_.a;

		if (false)
		{}
		else if (
			r_bf.count == 8 && r_bf.offset == 16 &&
			g_bf.count == 8 && g_bf.offset == 8 &&
			b_bf.count == 8 && b_bf.offset == 0 &&
			a_bf.count == 0)
		{
			bf_func = x8r8g8b8_to_a8b8g8r8;
		}
		else if (
			r_bf.count == 8 && r_bf.offset == 16 &&
			g_bf.count == 8 && g_bf.offset == 8 &&
			b_bf.count == 8 && b_bf.offset == 0 &&
			a_bf.count == 8 && a_bf.offset == 24)
		{
			bf_func = a8r8g8b8_to_a8b8g8r8;
		}
	}

	auto dst_bits = dst_bits_ + (is_top_down_ ? 0 : width_ * (height_ - 1));
	const auto dst_line_step = (is_top_down_ ? 0 : (-2 * width_));

	for (auto h = decltype(height_){}; h < height_; ++h)
	{
		for (auto w = decltype(width_){}; w < width_; ++w)
		{
			const auto src_pixel = read_32bpp_u32();
			const auto pixel = bf_func(bfs_, src_pixel);
			*dst_bits++ = pixel;
		}

		dst_bits += dst_line_step;
	}
}

void BmpImageDecoderImpl::decode_internal()
{
	if (width_ == 0 || height_ == 0)
	{
		return;
	}

	remain_bytes_ = bits_bytes_;
	remain_bytes_end_ = bits_bytes_end_;

	switch (bit_depth_)
	{
		case 1: decode_1bpp(); break;
		case 4: decode_4bpp(); break;
		case 8: decode_8bpp(); break;
		case 16: decode_16bpp(); break;
		case 24: decode_24bpp(); break;
		case 32: decode_32bpp(); break;
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported bit depth.");
	}
}

// ==========================================================================

BmpImageDecoder::~BmpImageDecoder() = default;

void BmpImageDecoder::decode(
	const void* src_data,
	int src_data_size,
	int& dst_width,
	int& dst_height,
	Rgba8Buffer& dst_buffer)
{
	BmpImageDecoderImpl{}.decode(src_data, src_data_size, dst_width, dst_height, dst_buffer);
}

} // bstone
