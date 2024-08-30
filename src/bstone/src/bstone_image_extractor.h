/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_IMAGE_EXTRACTOR_INCLUDED
#define BSTONE_IMAGE_EXTRACTOR_INCLUDED

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include "bstone_binary_writer.h"
#include "bstone_sprite_cache.h"

namespace bstone {

class ImageExtractor
{
public:
	ImageExtractor();

	void extract_vga_palette(const std::string& destination_dir);
	void extract_walls(const std::string& destination_dir);
	void extract_sprites(const std::string& destination_dir);

private:
	static constexpr auto max_bit_depth = 32;

	static constexpr auto palette_1bpp_size = 1 << 1;
	static constexpr auto palette_4bpp_size = 1 << 4;
	static constexpr auto palette_8bpp_size = 1 << 8;
	static constexpr auto max_palette_size = palette_8bpp_size;

	static constexpr auto max_width = 320;
	static constexpr auto max_height = 200;

	static constexpr auto wall_width = 64;
	static constexpr auto wall_height = 64;

	static constexpr auto sprite_width = Sprite::dimension;
	static constexpr auto sprite_height = Sprite::dimension;

	using Palette = std::array<std::uint32_t, max_palette_size>; // 0xAARRGGBB

	using ColorBuffer = std::vector<std::uint8_t>;
	using LineBuffer = std::array<std::uint8_t, max_width>;

private:
	static const std::uint8_t padding_bytes[3];

private:
	int sprite_count_{};
	std::string destination_dir_{};
	std::string destination_path_{};
	SpriteCache sprite_cache_{};
	int width_{};
	int height_{};
	int area_{};
	int bit_depth_{};
	int palette_size_{};
	int stride_{};
	Palette src_palette_{};
	Palette dst_palette_{};
	ColorBuffer color_buffer_{};
	LineBuffer line_buffer_{};
	std::uint8_t* colors8_;
	std::uint32_t* colors32_; // 0xAARRGGBB

private:
	void initialize_colors();
	void initialize_src_palette();
	void remap_indexed_image();

	void decode_default_palette() noexcept;
	void decode_wall_page(const std::uint8_t* src_colors) noexcept;
	void decode_sprite_page(const Sprite& sprite) noexcept;

	void save_bmp_rgb_palette(BinaryWriter& binary_writer);
	void save_bmp_rgbx_palette(BinaryWriter& binary_writer);
	void save_bmp_palette(BinaryWriter& binary_writer);

	void save_bmp_1bpp_bits(BinaryWriter& binary_writer);
	void save_bmp_4bpp_bits(BinaryWriter& binary_writer);
	void save_bmp_8bpp_bits(BinaryWriter& binary_writer);
	void save_bmp_32bpp_bits(BinaryWriter& binary_writer);

	void save_bmp(const std::string& path);
	void save_image(const std::string& name_prefix, int image_index);

	void extract_wall(int wall_index);
	void extract_sprite(int sprite_index);
};

} // namespace bstone

#endif // BSTONE_IMAGE_EXTRACTOR_INCLUDED
