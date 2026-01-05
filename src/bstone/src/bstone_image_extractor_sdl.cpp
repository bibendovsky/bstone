/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Image extractor (SDL)

#include "bstone_image_extractor.h"
#include "bstone_exception.h"
#include "bstone_fs.h"
#include "bstone_fs_utils.h"
#include "bstone_globals.h"
#include "bstone_rgb8.h"
#include "bstone_scope_exit.h"
#include "bstone_sprite_cache.h"
#include "id_ca.h"
#include "id_vh.h"
#include <cassert>
#include <cstdint>
#include <array>
#include <format>
#include <iterator>
#include <numeric>
#include <unordered_map>
#include <vector>
#include "SDL3/SDL_surface.h"

namespace bstone {

namespace {

class ImageExtractorImpl final : public ImageExtractor
{
public:
	ImageExtractorImpl();
	ImageExtractorImpl(const ImageExtractorImpl&) = delete;
	ImageExtractorImpl& operator=(const ImageExtractorImpl&) = delete;
	~ImageExtractorImpl() override = default;

private:
	constexpr static int wall_width = 64;
	constexpr static int wall_height = 64;
	constexpr static int wall_area = wall_width * wall_height;

	constexpr static int sprite_max_width = 64;
	constexpr static int sprite_max_height = 64;
	constexpr static int sprite_max_area = sprite_max_width * sprite_max_height;

	struct Color32
	{
		std::uint8_t r;
		std::uint8_t g;
		std::uint8_t b;
		std::uint8_t a;
	};

	using Palette = std::array<SDL_Color, 256>;
	using PaletteMap = std::unordered_map<int, int>;
	using Colors = std::vector<std::uint8_t>;
	using Colors32 = std::vector<SDL_Color>;

	constinit inline static const char* const file_ext = ".bmp";

	Logger& logger_;
	PageMgr& page_mgr_;
	SpriteCache sprite_cache_{};
	std::string dst_directory_{};
	int bit_depth_{};
	int width_{};
	int height_{};
	int palette_size_{};
	Palette bs_palette_{};
	Palette palette_{};
	PaletteMap palette_map_{};
	Colors colors_{};
	Colors32 colors32_{};

	void do_extract_vga_palette(const std::string& destination_dir) override;
	void do_extract_walls(const std::string& destination_dir) override;
	void do_extract_sprites(const std::string& destination_dir) override;

	[[noreturn]] static void fail_sdl_func(const char* sdl_func_name);
	void initialize_bs_palette();
	void save_image(const std::string& file_name_without_ext, int image_number);
	void impl_extract_palette();
	void impl_extract_wall(int wall_index);
	void impl_extract_sprite(int sprite_index);
};

// --------------------------------------

ImageExtractorImpl::ImageExtractorImpl()
	:
	logger_{*globals::logger},
	page_mgr_{*globals::page_mgr}
{
	palette_map_.reserve(256);
	initialize_bs_palette();
}

void ImageExtractorImpl::do_extract_vga_palette(const std::string& destination_dir)
{
	logger_.log_information();
	logger_.log_information("Extracting VGA palette.");
	logger_.log_information(std::format("Destination dir: {}", destination_dir).c_str());
	dst_directory_ = fs_utils::normalize_path(destination_dir);
	fs::create_directories(dst_directory_.c_str());
	impl_extract_palette();
	logger_.log_information("VGA palette has extracted.");
}

void ImageExtractorImpl::do_extract_walls(const std::string& destination_dir)
{
	const int wall_count = page_mgr_.get_wall_count();
	logger_.log_information();
	logger_.log_information("Extracting walls.");
	logger_.log_information(std::format("Destination dir: {}", destination_dir).c_str());
	logger_.log_information(std::format("Wall count: {}", wall_count).c_str());
	dst_directory_ = fs_utils::normalize_path(destination_dir);
	fs::create_directories(dst_directory_.c_str());
	for (int i = 0; i < wall_count; ++i)
	{
		impl_extract_wall(i);
	}
	logger_.log_information("Walls has extracted.");
}

void ImageExtractorImpl::do_extract_sprites(const std::string& destination_dir)
{
	const int sprite_count = std::max(page_mgr_.get_sprite_count(), 0);
	logger_.log_information();
	logger_.log_information("Extracting sprites.");
	logger_.log_information(std::format("Destination dir: {}", destination_dir).c_str());
	logger_.log_information(std::format("File count: {}", sprite_count).c_str());
	dst_directory_ = fs_utils::normalize_path(destination_dir);
	fs::create_directories(dst_directory_.c_str());
	for (int i = 1; i < sprite_count; ++i)
	{
		impl_extract_sprite(i);
	}
	logger_.log_information("Sprites has extracted.");
}

[[noreturn]] void ImageExtractorImpl::fail_sdl_func(const char* sdl_func_name)
{
	const std::string message = std::format("[{}] {}", sdl_func_name, SDL_GetError());
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

void ImageExtractorImpl::initialize_bs_palette()
{
	for (int i_color = 0; i_color < 256; ++i_color)
	{
		const std::uint8_t* const src_colors = &vgapal[i_color * 3];
		SDL_Color& dst_color = bs_palette_[i_color];
		dst_color.r = static_cast<std::uint8_t>((255 * src_colors[0]) / 63);
		dst_color.g = static_cast<std::uint8_t>((255 * src_colors[1]) / 63);
		dst_color.b = static_cast<std::uint8_t>((255 * src_colors[2]) / 63);
		dst_color.a = 255;
	}
}

void ImageExtractorImpl::save_image(const std::string& file_name_prefix, int image_number)
{
	bool has_palette = false;
	int pitch;
	SDL_PixelFormat sdl_surface_format;
	void* pixels;
	switch (bit_depth_)
	{
		case 8:
			has_palette = true;
			pitch = width_;
			sdl_surface_format = SDL_PIXELFORMAT_INDEX8;
			pixels = colors_.data();
			break;
		case 32:
			pitch = width_ * 4;
			sdl_surface_format = SDL_PIXELFORMAT_RGBA32;
			pixels = colors32_.data();
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown bit depth.");
	}
	const std::string file_path = fs_utils::append_path(
		dst_directory_,
		std::format(
			"{}{}{}",
			file_name_prefix,
			image_number >= 0 ? ca_make_padded_asset_number_string(image_number) : "",
			file_ext));
	SDL_Surface* const sdl_surface = SDL_CreateSurfaceFrom(width_, height_, sdl_surface_format, pixels, pitch);
	if (sdl_surface == nullptr)
	{
		fail_sdl_func("SDL_CreateSurfaceFrom");
	}
	const auto scope_exit = make_scope_exit(
		[&sdl_surface]()
		{
			SDL_DestroySurface(sdl_surface);
		});
	if (has_palette)
	{
		SDL_Palette* const sdl_palette = SDL_CreateSurfacePalette(sdl_surface);
		if (sdl_palette == nullptr)
		{
			fail_sdl_func("SDL_CreateSurfacePalette");
		}
		if (!SDL_SetPaletteColors(sdl_palette, palette_.data(), 0, palette_size_))
		{
			fail_sdl_func("SDL_SetPaletteColors");
		}
	}
	if (!SDL_SaveBMP(sdl_surface, file_path.c_str()))
	{
		fail_sdl_func("SDL_SaveBMP");
	}
}

void ImageExtractorImpl::impl_extract_palette()
{
	bit_depth_ = 8;
	width_ = 16;
	height_ = 16;
	palette_size_ = 256;
	palette_ = bs_palette_;
	colors_.clear();
	colors_.resize(256);
	std::iota(colors_.begin(), colors_.end(), std::uint8_t{});
	save_image("vga_palette", -1);
}

void ImageExtractorImpl::impl_extract_wall(int wall_index)
{
	const std::uint8_t* const wall_page = page_mgr_.get(wall_index);
	if (wall_page == nullptr)
	{
		const std::string message = std::format("Missing wall page. (index={})", wall_index);
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}
	bit_depth_ = 8;
	width_ = wall_width;
	height_ = wall_height;
	palette_size_ = 0;
	palette_map_.clear();
	colors_.clear();
	colors_.resize(wall_area);
	for (int x = 0; x < wall_width; ++x)
	{
		for (int y = 0; y < wall_height; ++y)
		{
			const int src_index = x * wall_height + y;
			const int src_color_index = wall_page[src_index];
			auto palette_map_iter = palette_map_.find(src_color_index);
			if (palette_map_iter == palette_map_.end())
			{
				palette_[palette_size_] = bs_palette_[src_color_index];
				palette_map_iter = palette_map_.emplace(src_color_index, palette_size_).first;
				++palette_size_;
			}
			const int dst_index = y * wall_width + x;
			const int dst_color_index = palette_map_iter->second;
			colors_[dst_index] = static_cast<std::uint8_t>(dst_color_index);
		}
	}
	save_image("wall_", wall_index);
}

void ImageExtractorImpl::impl_extract_sprite(int sprite_index)
{
	const bstone::Sprite& sprite = *sprite_cache_.cache(sprite_index);
	bit_depth_ = 32;
	width_ = sprite_max_width;
	height_ = sprite_max_height;
	palette_size_ = 0;
	palette_ = bs_palette_;
	colors32_.clear();
	colors32_.resize(sprite_max_area);
	const int left = sprite.get_left();
	const int right = sprite.get_right();
	const int top = sprite.get_top();
	const int bottom = sprite.get_bottom();
	for (int w = 0; w < width_; ++w)
	{
		const std::int16_t* column = nullptr;
		if (w >= left && w <= right)
		{
			column = sprite.get_column(w - left);
		}
		for (int h = 0; h < height_; ++h)
		{
			SDL_Color dst_color{};
			if (column != nullptr && h >= top && h <= bottom)
			{
				const int color_index = column[h - top];
				if (color_index >= 0)
				{
					dst_color = palette_[color_index];
				}
			}
			const int dst_index = (h * width_) + w;
			colors32_[dst_index] = dst_color;
		}
	}
	save_image("sprite_", sprite_index);
}

} // namespace

// ======================================

void ImageExtractor::extract_vga_palette(const std::string& destination_dir)
{
	do_extract_vga_palette(destination_dir);
}

void ImageExtractor::extract_walls(const std::string& destination_dir)
{
	do_extract_walls(destination_dir);
}

void ImageExtractor::extract_sprites(const std::string& destination_dir)
{
	do_extract_sprites(destination_dir);
}

// ======================================

using ImageExtractorUPtr = std::unique_ptr<ImageExtractor>;

ImageExtractorUPtr make_image_extractor()
{
	return std::make_unique<ImageExtractorImpl>();
}

} // namespace bstone
