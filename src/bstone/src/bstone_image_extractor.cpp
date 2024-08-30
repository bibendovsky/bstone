/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <algorithm>
#include <bitset>
#include <exception>
#include <memory>
#include "id_ca.h"
#include "id_heads.h"
#include "id_vh.h"
#include "bstone_binary_writer.h"
#include "bstone_bmp_image_common.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_fs_utils.h"
#include "bstone_globals.h"
#include "bstone_image_extractor.h"
#include "bstone_logger.h"
#include "bstone_memory_stream.h"

namespace bstone {

const std::uint8_t ImageExtractor::padding_bytes[3] = {};

ImageExtractor::ImageExtractor()
{
	initialize_colors();
	initialize_src_palette();
}

void ImageExtractor::extract_vga_palette(const std::string& destination_dir)
{
	globals::logger->log_information();
	globals::logger->log_information("<<< ================");
	globals::logger->log_information("Extracting VGA palette.");
	globals::logger->log_information(("Destination dir: \"" + destination_dir + "\"").c_str());

	destination_dir_ = fs_utils::normalize_path(destination_dir);

	decode_default_palette();

	const auto file_name = fs_utils::append_path(destination_dir_, "vga_palette.bmp");
	save_bmp(file_name);

	globals::logger->log_information(">>> ================");
}

void ImageExtractor::extract_walls(const std::string& destination_dir)
{
	const auto wall_count = globals::page_mgr->get_wall_count();

	globals::logger->log_information();
	globals::logger->log_information("<<< ================");
	globals::logger->log_information("Extracting walls.");
	globals::logger->log_information(("Destination dir: \"" + destination_dir + "\"").c_str());
	globals::logger->log_information(("File count: " + std::to_string(wall_count)).c_str());

	destination_dir_ = fs_utils::normalize_path(destination_dir);

	for (auto i = 0; i < wall_count; ++i)
	{
		extract_wall(i);
	}

	globals::logger->log_information(">>> ================");
}

void ImageExtractor::extract_sprites(const std::string& destination_dir)
{
	sprite_count_ = globals::page_mgr->get_sprite_count();

	if (sprite_count_ < 0)
	{
		sprite_count_ = 0;
	}

	globals::logger->log_information();
	globals::logger->log_information("<<< ================");
	globals::logger->log_information("Extracting sprites.");
	globals::logger->log_information(("Destination dir: \"" + destination_dir + "\"").c_str());
	globals::logger->log_information(("File count: " + std::to_string(sprite_count_)).c_str());

	destination_dir_ = fs_utils::normalize_path(destination_dir);

	for (auto i = 1; i < sprite_count_; ++i)
	{
		extract_sprite(i);
	}

	globals::logger->log_information(">>> ================");
}

void ImageExtractor::initialize_colors()
try {
	color_buffer_.resize(max_width * max_height * (((max_bit_depth + 7) / 8)) * 8);
	colors8_ = color_buffer_.data();
	colors32_ = reinterpret_cast<std::uint32_t*>(color_buffer_.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::initialize_src_palette()
try {
	auto src_colors = vgapal; // {0xRR, 0xGG, 0xBB}

	for (auto& dst_color : src_palette_)
	{
		const auto r = (255U * (*src_colors++)) / 63U;
		const auto g = (255U * (*src_colors++)) / 63U;
		const auto b = (255U * (*src_colors++)) / 63U;
		dst_color = 0xFF000000U | (r << 16) | (g << 8) | b;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::remap_indexed_image()
{
	using UsedColors = std::bitset<max_palette_size>;
	auto used_colors = UsedColors{};

	for (auto i = 0; i < area_; ++i)
	{
		used_colors.set(colors8_[i]);
	}

	const auto palette_size = used_colors.count();

	if (static_cast<int>(palette_size) == max_palette_size)
	{
		// Uses all palette colors. No need to re-map.
		//
		bit_depth_ = 8;
		dst_palette_ = src_palette_;
	}
	else
	{
		using IndexMap = std::array<unsigned char, max_palette_size>;
		auto index_map = IndexMap{};
		auto index = 0;

		for (auto i = 0; i < max_palette_size; ++i)
		{
			if (used_colors[i])
			{
				index_map[i] = static_cast<std::uint8_t>(index);
				dst_palette_[index] = src_palette_[i];
				++index;
			}
		}

		for (auto i = 0; i < area_; ++i)
		{
			const auto old_index = colors8_[i];
			const auto new_index = index_map[old_index];
			colors8_[i] = new_index;
		}

		if (palette_size <= (1U << 1))
		{
			bit_depth_ = 1;
		}
		else if (palette_size <= (1U << 4))
		{
			bit_depth_ = 4;
		}
		else
		{
			bit_depth_ = 8;
		}
	}

	palette_size_ = static_cast<int>(palette_size);
	stride_ = bmp::calculate_stride(width_, bit_depth_);
}

void ImageExtractor::decode_default_palette() noexcept
{
	width_ = 16;
	height_ = 16;
	area_ = width_ * height_;
	bit_depth_ = 0;
	palette_size_ = 0;
	stride_ = 0;

	for (auto i = 0; i < area_; ++i)
	{
		colors8_[i] = static_cast<std::uint8_t>(i);
	}

	remap_indexed_image();
}

void ImageExtractor::decode_wall_page(const std::uint8_t* src_colors) noexcept
{
	width_ = wall_width;
	height_ = wall_height;
	area_ = width_ * height_;
	bit_depth_ = 0;
	palette_size_ = 0;
	stride_ = 0;

	auto dst_colors = colors8_;

	for (auto w = 0; w < width_; ++w)
	{
		for (auto h = 0; h < height_; ++h)
		{
			const auto color_index = *src_colors++;
			const auto dst_index = (h * width_) + w;
			dst_colors[dst_index] = color_index;
		}
	}

	remap_indexed_image();
}

void ImageExtractor::decode_sprite_page(const Sprite& sprite) noexcept
{
	width_ = sprite_width;
	height_ = sprite_height;
	area_ = width_ * height_;
	bit_depth_ = 32;
	palette_size_ = 0;
	stride_ = bmp::calculate_stride(width_, bit_depth_);

	auto dst_colors = colors32_;
	const auto left = sprite.get_left();
	const auto right = sprite.get_right();
	const auto top = sprite.get_top();
	const auto bottom = sprite.get_bottom();

	for (auto w = 0; w < width_; ++w)
	{
		auto column = static_cast<const std::int16_t*>(nullptr);

		if (w >= left && w <= right)
		{
			column = sprite.get_column(w - left);
		}

		for (auto h = 0; h < height_; ++h)
		{
			auto dst_color = std::uint32_t{};

			if (column && h >= top && h <= bottom)
			{
				const auto color_index = column[h - top];

				if (color_index >= 0)
				{
					dst_color = src_palette_[color_index];
				}
			}

			const auto dst_index = (h * width_) + w;
			dst_colors[dst_index] = dst_color;
		}
	}
}

void ImageExtractor::save_bmp_rgb_palette(BinaryWriter& binary_writer)
try {
	struct Bgr
	{
		std::uint8_t b;
		std::uint8_t g;
		std::uint8_t r;
	};

	using PaletteBgr = std::array<Bgr, max_palette_size>;
	auto palette_bgr = PaletteBgr{};

	for (auto i = 0; i < palette_size_; ++i)
	{
		const auto& src_color = dst_palette_[i];
		auto& dst_color = palette_bgr[i];
		dst_color.b = src_color & 0xFFU;
		dst_color.g = (src_color >> 8) & 0xFFU;
		dst_color.r = (src_color >> 16) & 0xFFU;
	}

	binary_writer.get_stream().write_exactly(palette_bgr.data(), 3 * palette_size_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::save_bmp_rgbx_palette(BinaryWriter& binary_writer)
try {
	for (auto i = 0; i < palette_size_; ++i)
	{
		dst_palette_[i] = endian::to_little(dst_palette_[i]);
	}

	binary_writer.get_stream().write_exactly(dst_palette_.data(), 4 * palette_size_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::save_bmp_palette(BinaryWriter& binary_writer)
try {
	if (palette_size_ == (1 << bit_depth_))
	{
		save_bmp_rgb_palette(binary_writer);
	}
	else
	{
		save_bmp_rgbx_palette(binary_writer);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::save_bmp_1bpp_bits(BinaryWriter& binary_writer)
try {
	constexpr auto initial_mask = 0x80U;

	const auto line_size = ((width_ + 7) / 8) * 8;
	const auto padding_size = stride_ - line_size;
	auto src_colors = colors8_ + (width_ * (height_ - 1));

	for (auto h = 0; h < height_; ++h)
	{
		auto mask = 0U;
		auto line_byte = line_buffer_.data() - 1;

		for (auto w = 0; w < width_; ++w)
		{
			if (mask == 0U)
			{
				mask = initial_mask;
				*(++line_byte) = 0U;
			}

			if (src_colors[w] != 0U)
			{
				*line_byte |= mask;
			}
		}

		for (auto i = 0; i < padding_size; ++i)
		{
			*line_byte++ = 0U;
		}

		binary_writer.get_stream().write_exactly(line_buffer_.data(), line_size);
		src_colors -= width_;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::save_bmp_4bpp_bits(BinaryWriter& binary_writer)
try {
	auto src_colors = colors8_ + (width_ * (height_ - 1));

	for (auto h = 0; h < height_; ++h)
	{
		std::fill_n(line_buffer_.begin(), stride_, 0U);

		auto nibble_index = 0U;
		auto line_byte = line_buffer_.data();

		for (auto w = 0; w < width_; ++w)
		{
			const auto src_color = src_colors[w];
			*line_byte |= src_color << (4 * (nibble_index ^ 1U));

			if (nibble_index != 0U)
			{
				++line_byte;
			}

			nibble_index ^= 1U;
		}

		binary_writer.get_stream().write_exactly(line_buffer_.data(), stride_);
		src_colors -= width_;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::save_bmp_8bpp_bits(BinaryWriter& binary_writer)
try {
	const auto padding_size = stride_ - width_;
	auto src_colors = colors8_ + (width_ * (height_ - 1));

	for (auto h = 0; h < height_; ++h)
	{
		binary_writer.get_stream().write_exactly(src_colors, width_);
		binary_writer.get_stream().write_exactly(padding_bytes, padding_size);
		src_colors -= width_;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::save_bmp_32bpp_bits(BinaryWriter& binary_writer)
try {
	for (auto i = 0; i < area_; ++i)
	{
		*colors32_ = endian::to_little(*colors32_);
		++colors32_;
	}

	const auto bits_byte_count = stride_ * height_;
	binary_writer.get_stream().write_exactly(colors32_, bits_byte_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::save_bmp(const std::string& path)
try {
	auto file_stream = FileStream{
		path.c_str(),
		FileOpenFlags::create | FileOpenFlags::truncate | FileOpenFlags::write,
		FileShareMode::exclusive};

	auto binary_writer = BinaryWriter{file_stream};

	const auto is_core_header =
		palette_size_ == palette_1bpp_size ||
		palette_size_ == palette_4bpp_size ||
		palette_size_ == palette_8bpp_size;

	const auto is_bpp32 = (bit_depth_ == 32);

	const auto info_header_size =
		is_bpp32 ? bmp::bitmapv4header_size : (
			is_core_header ? bmp::bitmapcoreheader_size :
				bmp::bitmapinfoheader_size);

	const auto bits_offset =
		bmp::bitmapfileheader_size +
		info_header_size +
		(is_bpp32 ? 0 : (palette_size_ * (is_core_header ? 3 : 4)));

	const auto bits_byte_count = stride_ * height_;
	const auto file_size = bits_offset + bits_byte_count;
	const auto compression = (is_bpp32 ? bmp::bi_bitfields : bmp::bi_rgb);

	// -------------------------------------------------------------------------
	// BITMAPFILEHEADER

	// bfType
	binary_writer.write_u16(endian::to_little(std::uint16_t{bmp::type_bm}));

	// bfSize
	binary_writer.write_u32(endian::to_little(static_cast<std::uint32_t>(file_size)));

	// bfReserved1
	binary_writer.write_u16(endian::to_little(std::uint16_t{0}));

	// bfReserved2
	binary_writer.write_u16(endian::to_little(std::uint16_t{0}));

	// bfOffBits
	binary_writer.write_u32(endian::to_little(static_cast<std::uint32_t>(bits_offset)));

	// -------------------------------------------------------------------------
	// BITMAPCOREHEADER

	if (is_core_header)
	{
		// bcSize
		binary_writer.write_u32(endian::to_little(static_cast<std::uint32_t>(info_header_size)));

		// bcWidth
		binary_writer.write_u16(endian::to_little(static_cast<std::uint16_t>(width_)));

		// bcHeight
		binary_writer.write_u16(endian::to_little(static_cast<std::uint16_t>(height_)));

		// bcPlanes
		binary_writer.write_u16(endian::to_little(std::uint16_t{bmp::plane_count}));

		// bcBitCount
		binary_writer.write_u16(endian::to_little(static_cast<std::uint16_t>(bit_depth_)));
	}

	// -------------------------------------------------------------------------
	// BITMAPINFOHEADER

	if (!is_core_header)
	{
		// biSize
		binary_writer.write_u32(endian::to_little(static_cast<std::uint32_t>(info_header_size)));

		// biWidth
		binary_writer.write_s32(endian::to_little(width_));

		// biHeight
		binary_writer.write_s32(endian::to_little(is_bpp32 ? -height_ : height_));

		// biPlanes
		binary_writer.write_u16(endian::to_little(std::uint16_t{bmp::plane_count}));

		// biBitCount
		binary_writer.write_u16(endian::to_little(static_cast<std::uint16_t>(bit_depth_)));

		// biCompression
		binary_writer.write_u32(endian::to_little(compression));

		// biSizeImage
		binary_writer.write_u32(endian::to_little(static_cast<std::uint32_t>(bits_byte_count)));

		// biXPelsPerMeter
		binary_writer.write_s32(endian::to_little(std::int32_t{0}));

		// biYPelsPerMeter
		binary_writer.write_s32(endian::to_little(std::int32_t{0}));

		// biClrUsed
		binary_writer.write_u32(endian::to_little(static_cast<std::uint32_t>(palette_size_)));

		// biClrImportant
		binary_writer.write_u32(endian::to_little(std::uint32_t{0}));
	}

	// ----------------------------------------------------------------------
	// BITMAPV4HEADER

	if (is_bpp32)
	{
		constexpr std::uint8_t endpoints[bmp::ciexyztriple_size] = {};

		// bV4RedMask
		binary_writer.write_u32(endian::to_little(0x00FF0000U));

		// bV4GreenMask
		binary_writer.write_u32(endian::to_little(0x0000FF00U));

		// bV4BlueMask
		binary_writer.write_u32(endian::to_little(0x000000FFU));

		// bV4AlphaMask
		binary_writer.write_u32(endian::to_little(0xFF000000U));

		// bV4CSType
		binary_writer.write_u32(endian::to_little(bmp::lcs_calibrated_rgb));

		// bV4Endpoints
		binary_writer.get_stream().write(endpoints, static_cast<std::intptr_t>(sizeof(decltype(endpoints))));

		// bV4GammaRed
		binary_writer.write_u32(endian::to_little(0));

		// bV4GammaGreen
		binary_writer.write_u32(endian::to_little(0));

		// bV4GammaBlue
		binary_writer.write_u32(endian::to_little(0));
	}

	// -------------------------------------------------------------------------
	// Palette

	if (bit_depth_ <= 8)
	{
		save_bmp_palette(binary_writer);
	}

	// -------------------------------------------------------------------------
	// Colors

	switch (bit_depth_)
	{
		case 1: save_bmp_1bpp_bits(binary_writer); break;
		case 4: save_bmp_4bpp_bits(binary_writer); break;
		case 8: save_bmp_8bpp_bits(binary_writer); break;
		case 32: save_bmp_32bpp_bits(binary_writer); break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown bit depth.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::save_image(const std::string& name_prefix, int image_index)
try {
	const auto& wall_index_string = ca_make_padded_asset_number_string(image_index);

	const auto& file_name = fs_utils::append_path(
		destination_dir_,
		name_prefix + wall_index_string + ".bmp");

	save_bmp(file_name);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::extract_wall(int wall_index)
try {
	const auto wall_page = globals::page_mgr->get(wall_index);

	if (wall_page == nullptr)
	{
		const auto error_message = std::string{} + "No wall page #" + std::to_string(wall_index) + ".";
		BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
	}

	decode_wall_page(wall_page);
	save_image("wall_", wall_index);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void ImageExtractor::extract_sprite(int sprite_index)
try {
	const auto cache_sprite_index = sprite_index;
	const auto sprite = sprite_cache_.cache(cache_sprite_index);
	decode_sprite_page(*sprite);
	save_image("sprite_", cache_sprite_index);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
