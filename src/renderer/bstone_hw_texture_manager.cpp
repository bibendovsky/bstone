/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// Renderer texture manager (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_hw_texture_manager.h"

#include <cassert>
#include <array>
#include <unordered_map>

#include "xbrz.h"

#include "id_pm.h"
#include "id_vl.h"

#include "bstone_exception.h"
#include "bstone_missing_sprite_64x64_image.h"
#include "bstone_missing_wall_64x64_image.h"
#include "bstone_ref_values.h"
#include "bstone_renderer.h"
#include "bstone_rgb_palette.h"
#include "bstone_sprite_cache.h"
#include "bstone_detail_renderer_utils.h"


namespace bstone
{


// ==========================================================================
// GenericHwTextureManager
//

class GenericHwTextureManager final :
	public HwTextureManager
{
public:
	GenericHwTextureManager(
		RendererPtr renderer,
		SpriteCachePtr sprite_cache);

	~GenericHwTextureManager() override;


	int upscale_filter_get_min_factor(
		const HwTextureManagerUpscaleFilterKind upscale_filter_kind) const override;

	int upscale_filter_get_max_factor(
		const HwTextureManagerUpscaleFilterKind upscale_filter_kind) const override;

	HwTextureManagerUpscaleFilterKind upscale_filter_get_kind() const noexcept override;

	int upscale_filter_get_factor() const noexcept override;

	void upscale_filter_set(
		const HwTextureManagerUpscaleFilterKind upscale_filter,
		const int upscale_filter_factor) override;


	void cache_begin() override;

	void cache_end() override;

	void cache_purge() override;


	void wall_cache(
		const int id) override;

	RendererTexture2dPtr wall_get(
		const int id) const override;


	void sprite_cache(
		const int id) override;

	RendererTexture2dPtr sprite_get(
		const int id) const override;


	void ui_destroy() override;

	void ui_create(
		const std::uint8_t* const indexed_pixels,
		const bool* const indexed_alphas,
		const R8g8b8a8PaletteCPtr indexed_palette) override;

	void ui_update() override;

	RendererTexture2dPtr ui_get() const override;


	void solid_1x1_destroy(
		const HwTextureManagerSolid1x1Id id) override;

	void solid_1x1_create(
		const HwTextureManagerSolid1x1Id id) override;

	void solid_1x1_update(
		const HwTextureManagerSolid1x1Id id,
		const R8g8b8a8 color) override;

	RendererTexture2dPtr solid_1x1_get(
		const HwTextureManagerSolid1x1Id id) const override;


	void device_on_reset() override;


private:
	static constexpr auto wall_dimension = 64;

	static constexpr auto max_walls = 256;
	static constexpr auto max_sprites = 1'024;


	using GenerationId = unsigned int;

	static constexpr auto invalid_generation_id = GenerationId{};
	static constexpr auto first_generation_id = GenerationId{1};


	enum class ImageKind
	{
		sprite,
		wall,
	}; // ImageKind

	struct Texture2dProperties
	{
		RendererPixelFormat image_pixel_format_;

		bool is_npot_;

		int width_;
		int height_;

		int upscale_width_;
		int upscale_height_;

		int actual_width_;
		int actual_height_;

		bool is_generate_mipmaps_;
		int mipmap_count_;

		bool indexed_is_column_major_;
		const std::uint8_t* indexed_pixels_;
		R8g8b8a8PaletteCPtr indexed_palette_;
		const bool* indexed_alphas_;

		SpriteCPtr indexed_sprite_;

		R8g8b8a8CPtr rgba_pixels_;
	}; // Texture2dProperties

	struct Texture2dItem
	{
		GenerationId generation_id_;
		Texture2dProperties properties_;
		RendererTexture2dPtr texture_2d_;
	}; // Texture2dItem

	using IdToTexture2dMap = std::unordered_map<int, Texture2dItem>;

	struct Solid1x1Item
	{
		R8g8b8a8 color_;
		Texture2dProperties properties_;
		RendererTexture2dPtr texture_2d_;
	}; // Solid1x1Item

	using Solid1x1Items = std::array<Solid1x1Item, static_cast<std::size_t>(HwTextureManagerSolid1x1Id::count_)>;


	RendererPtr renderer_;
	SpriteCachePtr sprite_cache_;

	HwTextureManagerUpscaleFilterKind upscale_filter_kind_;
	int upscale_filter_factor_;

	bool is_caching_;
	GenerationId generation_id_;

	IdToTexture2dMap wall_map_;
	IdToTexture2dMap sprite_map_;

	Texture2dItem missing_sprite_texture_2d_item_;
	Texture2dItem missing_wall_texture_2d_item_;

	Texture2dItem ui_t2d_item_;

	detail::RendererUtils::TextureBuffer mipmap_buffer_;
	detail::RendererUtils::TextureBuffer upscale_buffer_;

	Solid1x1Items solid_1x1_items_;


	static void validate_upscale_filter(
		const HwTextureManagerUpscaleFilterKind upscale_filter_kind,
		const int upscale_filter_factor);


	void validate_image_source_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_image_pixel_format_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_dimensions_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_mipmap_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_common_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_indexed_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_indexed_sprite_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_rgba_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_source_texture_2d_properties(
		const Texture2dProperties& properties);

	void validate_texture_2d_properties(
		const Texture2dProperties& properties);

	void set_common_texture_2d_properties(
		Texture2dProperties& properties);


	void recreate_indexed_resources();

	void uninitialize();

	void initialize(
		RendererPtr renderer,
		SpriteCachePtr sprite_cache);

	void uninitialize_internal();


	void upscale_xbrz(
		const Texture2dProperties& properties);

	void upscale(
		const Texture2dProperties& properties);


	Texture2dItem create_texture(
		const Texture2dProperties& properties);

	void update_mipmaps(
		const Texture2dProperties& properties,
		RendererTexture2dPtr texture_2d);


	void destroy_missing_sprite_texture();

	void create_missing_sprite_texture();


	void destroy_missing_wall_texture();

	void create_missing_wall_texture();


	Texture2dItem wall_create_texture(
		const int wall_id);

	Texture2dItem sprite_create_texture(
		const int sprite_id);


	void initialize_internal(
		RendererPtr renderer,
		SpriteCachePtr sprite_cache);

	void cache_purge(
		IdToTexture2dMap& map);

	RendererTexture2dPtr get_texture_2d(
		const ImageKind image_kind,
		const int id,
		const IdToTexture2dMap& map) const;

	void solid_1x1_destroy_all();

	static int solid_1x1_get_index(
		const HwTextureManagerSolid1x1Id id);

	static int solid_1x1_get_updateable_index(
		const HwTextureManagerSolid1x1Id id);

	static R8g8b8a8 solid_1x1_get_default_color(
		const HwTextureManagerSolid1x1Id id);

	static int upscale_filter_get_min_factor_internal(
		const HwTextureManagerUpscaleFilterKind upscale_filter_kind);

	static int upscale_filter_get_max_factor_internal(
		const HwTextureManagerUpscaleFilterKind upscale_filter_kind);

	static int upscale_filter_clamp_factor(
		const HwTextureManagerUpscaleFilterKind upscale_filter_kind,
		const int upscale_filter_factor);
}; // Detail

using HwTextureManagerImplUPtr = std::unique_ptr<GenericHwTextureManager>;


GenericHwTextureManager::GenericHwTextureManager(
	RendererPtr renderer,
	SpriteCachePtr sprite_cache)
	:
	renderer_{},
	upscale_filter_kind_{},
	upscale_filter_factor_{},
	sprite_cache_{},
	is_caching_{},
	generation_id_{},
	wall_map_{},
	sprite_map_{},
	missing_sprite_texture_2d_item_{},
	missing_wall_texture_2d_item_{},
	ui_t2d_item_{},
	mipmap_buffer_{},
	upscale_buffer_{},
	solid_1x1_items_{}
{
	initialize(renderer, sprite_cache);
}

GenericHwTextureManager::~GenericHwTextureManager()
{
	uninitialize_internal();
}

void GenericHwTextureManager::uninitialize()
{
	uninitialize_internal();
}

void GenericHwTextureManager::recreate_indexed_resources()
{
	// Sprites.
	//
	for (auto& sprite_item : sprite_map_)
	{
		const auto sprite_id = sprite_item.first;
		auto& texture_2d_item = sprite_item.second;

		if (texture_2d_item.properties_.rgba_pixels_)
		{
			continue;
		}

		renderer_->texture_2d_destroy(texture_2d_item.texture_2d_);
		texture_2d_item.texture_2d_ = nullptr;

		texture_2d_item = sprite_create_texture(sprite_id);
		texture_2d_item.generation_id_ = generation_id_;
	}

	// Walls.
	//
	for (auto& wall_item : wall_map_)
	{
		const auto wall_id = wall_item.first;
		auto& texture_2d_item = wall_item.second;

		if (texture_2d_item.properties_.rgba_pixels_)
		{
			continue;
		}

		renderer_->texture_2d_destroy(texture_2d_item.texture_2d_);
		texture_2d_item.texture_2d_ = nullptr;

		texture_2d_item = wall_create_texture(wall_id);
		texture_2d_item.generation_id_ = generation_id_;
	}

	// UI texture.
	//
	{
		ui_destroy();

		ui_create(
			ui_t2d_item_.properties_.indexed_pixels_,
			ui_t2d_item_.properties_.indexed_alphas_,
			ui_t2d_item_.properties_.indexed_palette_);
	}
}

int GenericHwTextureManager::upscale_filter_get_min_factor(
	const HwTextureManagerUpscaleFilterKind upscale_filter_kind) const
{
	return upscale_filter_get_min_factor_internal(upscale_filter_kind);
}

int GenericHwTextureManager::upscale_filter_get_max_factor(
	const HwTextureManagerUpscaleFilterKind upscale_filter_kind) const
{
	return upscale_filter_get_max_factor_internal(upscale_filter_kind);
}

HwTextureManagerUpscaleFilterKind GenericHwTextureManager::upscale_filter_get_kind() const noexcept
{
	return upscale_filter_kind_;
}

int GenericHwTextureManager::upscale_filter_get_factor() const noexcept
{
	return upscale_filter_factor_;
}

void GenericHwTextureManager::upscale_filter_set(
	const HwTextureManagerUpscaleFilterKind upscale_filter_kind,
	const int upscale_filter_factor)
{
	const auto clamped_upscale_filter_factor = upscale_filter_clamp_factor(
		upscale_filter_kind,
		upscale_filter_factor
	);

	if (upscale_filter_kind_ == upscale_filter_kind &&
		upscale_filter_factor_ == clamped_upscale_filter_factor)
	{
		return;
	}

	validate_upscale_filter(upscale_filter_kind, clamped_upscale_filter_factor);

	upscale_filter_kind_ = upscale_filter_kind;
	upscale_filter_factor_ = clamped_upscale_filter_factor;

	recreate_indexed_resources();
}

void GenericHwTextureManager::cache_begin()
{
	if (is_caching_)
	{
		throw Exception{"Already caching."};
	}

	is_caching_ = true;

	++generation_id_;

	if (generation_id_ == invalid_generation_id)
	{
		generation_id_ = first_generation_id;
	}
}

void GenericHwTextureManager::cache_end()
{
	if (!is_caching_)
	{
		throw Exception{"Not caching."};
	}

	is_caching_ = false;
}

void GenericHwTextureManager::cache_purge()
{
	if (is_caching_)
	{
		throw Exception{"Caching is active."};
	}

	cache_purge(wall_map_);
	cache_purge(sprite_map_);
}

void GenericHwTextureManager::wall_cache(
	const int id)
{
	if (id < 0 || id >= max_walls)
	{
		throw Exception{"Id out of range."};
	}

	auto wall_it = wall_map_.find(id);

	if (wall_it != wall_map_.end())
	{
		wall_it->second.generation_id_ = generation_id_;

		return;
	}

	auto texture_2d_item = wall_create_texture(id);

	texture_2d_item.generation_id_ = generation_id_;

	wall_map_[id] = texture_2d_item;
}

RendererTexture2dPtr GenericHwTextureManager::wall_get(
	const int id) const
{
	if (id < 0 || id >= max_walls)
	{
		throw Exception{"Id out of range."};
	}

	return get_texture_2d(ImageKind::wall, id, wall_map_);
}

void GenericHwTextureManager::sprite_cache(
	const int id)
{
	if (id <= 0 || id >= max_sprites)
	{
		throw Exception{"Id out of range."};
	}

	auto sprite_it = sprite_map_.find(id);

	if (sprite_it != sprite_map_.end())
	{
		sprite_it->second.generation_id_ = generation_id_;

		return;
	}

	auto texture_2d_item = sprite_create_texture(id);

	texture_2d_item.generation_id_ = generation_id_;

	sprite_map_[id] = texture_2d_item;
}

RendererTexture2dPtr GenericHwTextureManager::sprite_get(
	const int id) const
{
	if (id <= 0 || id >= max_sprites)
	{
		throw Exception{"Id out of range."};
	}

	return get_texture_2d(ImageKind::sprite, id, sprite_map_);
}

void GenericHwTextureManager::ui_destroy()
{
	if (ui_t2d_item_.texture_2d_ == nullptr)
	{
		return;
	}

	renderer_->texture_2d_destroy(ui_t2d_item_.texture_2d_);
	ui_t2d_item_.texture_2d_ = nullptr;
}

void GenericHwTextureManager::ui_create(
	const std::uint8_t* const indexed_pixels,
	const bool* const indexed_alphas,
	const R8g8b8a8PaletteCPtr indexed_palette)
{
	if (ui_t2d_item_.texture_2d_ != nullptr)
	{
		throw Exception{"Already created."};
	}

	if (indexed_pixels == nullptr)
	{
		throw Exception{"Null indexed pixels."};
	}

	if (indexed_alphas == nullptr)
	{
		throw Exception{"Null indexed alphas."};
	}

	if (indexed_palette == nullptr)
	{
		throw Exception{"Null indexed palette."};
	}

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = RendererPixelFormat::r8g8b8a8_unorm;
	param.width_ = ::vga_ref_width;
	param.height_ = ::vga_ref_height;
	param.mipmap_count_ = 1;
	param.indexed_pixels_ = indexed_pixels;
	param.indexed_palette_ = indexed_palette;
	param.indexed_alphas_ = indexed_alphas;

	auto texture_2d_item = create_texture(param);

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	ui_t2d_item_ = texture_2d_item;
}

void GenericHwTextureManager::ui_update()
{
	update_mipmaps(ui_t2d_item_.properties_, ui_t2d_item_.texture_2d_);
}

RendererTexture2dPtr GenericHwTextureManager::ui_get() const
{
	return ui_t2d_item_.texture_2d_;
}

void GenericHwTextureManager::solid_1x1_destroy(
	const HwTextureManagerSolid1x1Id id)
{
	const auto index = solid_1x1_get_index(id);

	auto& item = solid_1x1_items_[index];

	if (item.texture_2d_ == nullptr)
	{
		return;
	}

	item.color_ = {};

	renderer_->texture_2d_destroy(item.texture_2d_);
	item.texture_2d_ = nullptr;
}

void GenericHwTextureManager::solid_1x1_create(
	const HwTextureManagerSolid1x1Id id)
{
	const auto index = solid_1x1_get_index(id);

	const auto default_color = solid_1x1_get_default_color(id);
	const auto has_alpha = (default_color.a_ < 0xFF);

	const auto image_pixel_format = (has_alpha ? bstone::RendererPixelFormat::r8g8b8a8_unorm : bstone::RendererPixelFormat::r8g8b8_unorm);

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = image_pixel_format;
	param.width_ = 1;
	param.height_ = 1;
	param.mipmap_count_ = 1;
	param.rgba_pixels_ = &default_color;

	auto texture_2d_item = create_texture(param);

	auto& item = solid_1x1_items_[index];
	item.color_ = default_color;
	item.properties_ = texture_2d_item.properties_;
	item.texture_2d_ = texture_2d_item.texture_2d_;

	update_mipmaps(item.properties_, texture_2d_item.texture_2d_);
}

void GenericHwTextureManager::solid_1x1_update(
	const HwTextureManagerSolid1x1Id id,
	const R8g8b8a8 color)
{
	const auto index = solid_1x1_get_updateable_index(id);

	auto& item = solid_1x1_items_[index];
	item.color_ = color;

	auto param = RendererTexture2dUpdateParam{};
	param.rgba_pixels_ = &item.color_;

	item.texture_2d_->update(param);
}

RendererTexture2dPtr GenericHwTextureManager::solid_1x1_get(
	const HwTextureManagerSolid1x1Id id) const
{
	const auto index = solid_1x1_get_index(id);

	if (index < 0)
	{
		assert(!"Invalid index.");

		return nullptr;
	}

	return solid_1x1_items_[index].texture_2d_;
}

void GenericHwTextureManager::device_on_reset()
{
	// Missing sprite texture.
	//
	{
		destroy_missing_sprite_texture();
		create_missing_sprite_texture();
	}

	// Missing wall texture.
	//
	{
		destroy_missing_wall_texture();
		create_missing_wall_texture();
	}

	// Sprites.
	//
	for (auto& sprite_item : sprite_map_)
	{
		const auto sprite_id = sprite_item.first;
		auto& texture_2d_item = sprite_item.second;

		renderer_->texture_2d_destroy(texture_2d_item.texture_2d_);
		texture_2d_item.texture_2d_ = nullptr;

		texture_2d_item = sprite_create_texture(sprite_id);
		texture_2d_item.generation_id_ = generation_id_;
	}

	// Walls.
	//
	for (auto& wall_item : wall_map_)
	{
		const auto wall_id = wall_item.first;
		auto& texture_2d_item = wall_item.second;

		renderer_->texture_2d_destroy(texture_2d_item.texture_2d_);
		texture_2d_item.texture_2d_ = nullptr;

		texture_2d_item = wall_create_texture(wall_id);
		texture_2d_item.generation_id_ = generation_id_;
	}

	// UI texture.
	//
	{
		ui_destroy();

		ui_create(
			ui_t2d_item_.properties_.indexed_pixels_,
			ui_t2d_item_.properties_.indexed_alphas_,
			ui_t2d_item_.properties_.indexed_palette_);
	}

	// Solid 1x1 textures.
	//
	for (int i = 0; i < static_cast<int>(HwTextureManagerSolid1x1Id::count_); ++i)
	{
		const auto id = static_cast<HwTextureManagerSolid1x1Id>(i);

		solid_1x1_destroy(id);
		solid_1x1_create(id);
	}
}

void GenericHwTextureManager::initialize(
	RendererPtr renderer,
	SpriteCachePtr sprite_cache)
{
	initialize_internal(renderer, sprite_cache);
}

void GenericHwTextureManager::uninitialize_internal()
{
	generation_id_ = invalid_generation_id;

	// Wall map.
	//
	for (auto& wall_item : wall_map_)
	{
		const auto texture_2d = wall_item.second.texture_2d_;
		wall_item.second.texture_2d_ = nullptr;

		renderer_->texture_2d_destroy(texture_2d);
	}

	wall_map_.clear();

	// Sprite map.
	//
	for (auto& sprite_item : sprite_map_)
	{
		const auto texture_2d = sprite_item.second.texture_2d_;
		sprite_item.second.texture_2d_ = nullptr;

		renderer_->texture_2d_destroy(texture_2d);
	}

	sprite_map_.clear();

	// "Missing" textures.
	//
	destroy_missing_sprite_texture();
	destroy_missing_wall_texture();

	// UI texture.
	//
	ui_destroy();

	// Solid 1x1 textures.
	//
	solid_1x1_destroy_all();

	renderer_ = nullptr;
	sprite_cache_ = nullptr;
	mipmap_buffer_.clear();
}

void GenericHwTextureManager::validate_upscale_filter(
	const HwTextureManagerUpscaleFilterKind upscale_filter_kind,
	const int upscale_filter_factor)
{
	switch (upscale_filter_kind)
	{
		case HwTextureManagerUpscaleFilterKind::none:
		case HwTextureManagerUpscaleFilterKind::xbrz:
			return;

		default:
			throw Exception{"Unsupported upscale filter kind."};
	}

	const auto min_factor = upscale_filter_get_min_factor_internal(upscale_filter_kind);
	const auto max_factor = upscale_filter_get_max_factor_internal(upscale_filter_kind);

	if (upscale_filter_factor < min_factor ||
		upscale_filter_factor > max_factor)
	{
		throw Exception{"Upscale factor out of range."};
	}
}

void GenericHwTextureManager::validate_image_source_texture_2d_properties(
	const Texture2dProperties& properties)
{
	auto source_count = 0;

	if (properties.indexed_pixels_ != nullptr)
	{
		++source_count;
	}

	if (properties.indexed_sprite_ != nullptr)
	{
		++source_count;
	}

	if (properties.rgba_pixels_ != nullptr)
	{
		++source_count;
	}

	if (source_count == 0)
	{
		throw Exception{"No image source."};
	}

	if (source_count > 1)
	{
		throw Exception{"Multiple image source."};
	}
}

void GenericHwTextureManager::validate_image_pixel_format_texture_2d_properties(
	const Texture2dProperties& properties)
{
	switch (properties.image_pixel_format_)
	{
		case RendererPixelFormat::r8g8b8_unorm:
		case RendererPixelFormat::r8g8b8a8_unorm:
			return;

		default:
			throw Exception{"Invalid pixel format."};
	}
}

void GenericHwTextureManager::validate_dimensions_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.width_ <= 0)
	{
		throw Exception{"Invalid width."};
	}

	if (properties.height_ <= 0)
	{
		throw Exception{"Invalid height."};
	}
}

void GenericHwTextureManager::validate_mipmap_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.mipmap_count_ <= 0 ||
		properties.mipmap_count_ > detail::RendererUtils::get_max_mipmap_count())
	{
		throw Exception{"Mipmap count out of range."};
	}
}

void GenericHwTextureManager::validate_common_texture_2d_properties(
	const Texture2dProperties& properties)
{
	validate_image_source_texture_2d_properties(properties);
	validate_image_pixel_format_texture_2d_properties(properties);
	validate_dimensions_texture_2d_properties(properties);
	validate_mipmap_texture_2d_properties(properties);
}

void GenericHwTextureManager::validate_indexed_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.indexed_pixels_ == nullptr)
	{
		throw Exception{"Null indexed image source."};
	}

	if (properties.indexed_palette_ == nullptr)
	{
		throw Exception{"Null indexed palette."};
	}
}

void GenericHwTextureManager::validate_indexed_sprite_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.indexed_sprite_ == nullptr)
	{
		throw Exception{"Null indexed sprite."};
	}

	if (properties.indexed_palette_ == nullptr)
	{
		throw Exception{"Null indexed palette."};
	}
}

void GenericHwTextureManager::validate_rgba_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.rgba_pixels_ == nullptr)
	{
		throw Exception{"Null RGBA image."};
	}
}

void GenericHwTextureManager::validate_source_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.indexed_pixels_ != nullptr)
	{
		validate_indexed_texture_2d_properties(properties);
	}
	else if (properties.indexed_sprite_ != nullptr)
	{
		validate_indexed_sprite_texture_2d_properties(properties);
	}
	else if (properties.rgba_pixels_ != nullptr)
	{
		validate_rgba_texture_2d_properties(properties);
	}
	else
	{
		throw Exception{"No image source."};
	}
}

void GenericHwTextureManager::validate_texture_2d_properties(
	const Texture2dProperties& properties)
{
	validate_common_texture_2d_properties(properties);
	validate_source_texture_2d_properties(properties);
}

void GenericHwTextureManager::set_common_texture_2d_properties(
	Texture2dProperties& properties)
{
	const auto& device_features = renderer_->device_get_features();

	auto upscale_width = properties.width_;
	auto upscale_height = properties.height_;

	if (properties.indexed_pixels_ || properties.indexed_sprite_)
	{
		switch (upscale_filter_kind_)
		{
			case HwTextureManagerUpscaleFilterKind::xbrz:
				upscale_width *= upscale_filter_factor_;
				upscale_height *= upscale_filter_factor_;
				break;

			default:
				break;
		}
	}

	auto actual_width = upscale_width;
	auto actual_height = upscale_height;

	const auto is_width_pot = detail::RendererUtils::is_pot_value(actual_width);
	const auto is_height_pot = detail::RendererUtils::is_pot_value(actual_height);

	const auto is_npot = (!is_width_pot || !is_height_pot);
	const auto has_hw_npot = (!is_npot || (is_npot && device_features.npot_is_available_));

	if (has_hw_npot)
	{
		actual_width = std::min(actual_width, device_features.max_texture_dimension_);
		actual_height = std::min(actual_height, device_features.max_texture_dimension_);
	}
	else
	{
		actual_width = detail::RendererUtils::find_nearest_pot_value(actual_width);
		actual_width = std::min(actual_width, device_features.max_texture_dimension_);

		actual_height = detail::RendererUtils::find_nearest_pot_value(actual_height);
		actual_height = std::min(actual_height, device_features.max_texture_dimension_);
	}

	properties.upscale_width_ = upscale_width;
	properties.upscale_height_ = upscale_height;

	properties.actual_width_ = actual_width;
	properties.actual_height_ = actual_height;

	properties.is_npot_ = is_npot;
}

void GenericHwTextureManager::upscale_xbrz(
	const Texture2dProperties& properties)
{
	const auto area = properties.width_ * properties.height_;
	const auto upscale_area = properties.upscale_width_ * properties.upscale_height_;

	if (mipmap_buffer_.size() < area)
	{
		mipmap_buffer_.resize(area);
	}

	if (properties.indexed_pixels_)
	{
		detail::RendererUtils::indexed_to_rgba(
			properties.width_,
			properties.height_,
			properties.indexed_is_column_major_,
			properties.indexed_pixels_,
			*properties.indexed_palette_,
			properties.indexed_alphas_,
			mipmap_buffer_
		);
	}
	else if (properties.indexed_sprite_)
	{
		detail::RendererUtils::indexed_sprite_to_rgba_pot(
			*properties.indexed_sprite_,
			*properties.indexed_palette_,
			mipmap_buffer_
		);
	}
	else
	{
		throw Exception{"Unsupported image source."};
	}

	if (upscale_buffer_.size() < upscale_area)
	{
		upscale_buffer_.resize(upscale_area);
	}

	xbrz::scale(
		static_cast<std::size_t>(upscale_filter_factor_),
		reinterpret_cast<const std::uint32_t*>(mipmap_buffer_.data()),
		reinterpret_cast<std::uint32_t*>(upscale_buffer_.data()),
		properties.width_,
		properties.height_,
		xbrz::ColorFormat::ARGB
	);
}

void GenericHwTextureManager::upscale(
	const Texture2dProperties& properties)
{
	if (properties.upscale_width_ == properties.width_ &&
		properties.upscale_height_ == properties.height_)
	{
		return;
	}

	switch (upscale_filter_kind_)
	{
		case HwTextureManagerUpscaleFilterKind::xbrz:
			upscale_xbrz(properties);

		default:
			break;
	}
}

GenericHwTextureManager::Texture2dItem GenericHwTextureManager::create_texture(
	const Texture2dProperties& properties)
{
	validate_texture_2d_properties(properties);

	auto new_properties = properties;

	set_common_texture_2d_properties(new_properties);

	// Create texture object.
	//
	auto param = RendererTexture2dCreateParam{};
	param.storage_pixel_format_ = new_properties.image_pixel_format_;
	param.width_ = new_properties.actual_width_;
	param.height_ = new_properties.actual_height_;
	param.mipmap_count_ = new_properties.mipmap_count_;

	auto texture_2d = renderer_->texture_2d_create(param);

	update_mipmaps(new_properties, texture_2d);

	// Return the result.
	//
	auto result = Texture2dItem{};

	result.properties_ = new_properties;
	result.texture_2d_ = texture_2d;

	return result;
}

void GenericHwTextureManager::update_mipmaps(
	const Texture2dProperties& properties,
	RendererTexture2dPtr texture_2d)
{
	upscale(properties);

	const auto is_upscale = (
		properties.upscale_width_ != properties.width_ &&
		properties.upscale_height_ != properties.height_);

	const auto& device_features = renderer_->device_get_features();

	const auto npot_is_available = device_features.npot_is_available_;

	const auto max_subbuffer_size = properties.actual_width_ * properties.actual_height_;

	auto max_buffer_size = max_subbuffer_size;

	const auto is_manual_mipmaps =
		properties.is_generate_mipmaps_ &&
		properties.mipmap_count_ > 1 &&
		!device_features.mipmap_is_available_;

	if (is_manual_mipmaps)
	{
		max_buffer_size *= 2;
	}

	if (static_cast<int>(mipmap_buffer_.size()) < max_buffer_size)
	{
		mipmap_buffer_.resize(max_buffer_size);
	}

	auto texture_subbuffer_0 = &mipmap_buffer_[0];
	auto texture_subbuffer_1 = R8g8b8a8Ptr{};

	if (is_manual_mipmaps)
	{
		texture_subbuffer_1 = &mipmap_buffer_[max_subbuffer_size];
	}

	auto is_set_subbuffer_0 = false;

	if (is_upscale)
	{
		if (properties.is_npot_ && !npot_is_available)
		{
			detail::RendererUtils::rgba_npot_to_rgba_pot(
				properties.upscale_width_,
				properties.upscale_height_,
				properties.actual_width_,
				properties.actual_height_,
				upscale_buffer_.data(),
				mipmap_buffer_
			);
		}
		else
		{
			// Don't copy the base mipmap into a buffer.

			is_set_subbuffer_0 = true;

			texture_subbuffer_0 = upscale_buffer_.data();
		}
	}
	else if (properties.rgba_pixels_)
	{
		if (properties.is_npot_ && !npot_is_available)
		{
			detail::RendererUtils::rgba_npot_to_rgba_pot(
				properties.width_,
				properties.height_,
				properties.actual_width_,
				properties.actual_height_,
				properties.rgba_pixels_,
				mipmap_buffer_
			);
		}
		else
		{
			// Don't copy the base mipmap into a buffer.

			is_set_subbuffer_0 = true;

			texture_subbuffer_0 = const_cast<R8g8b8a8Ptr>(properties.rgba_pixels_);
		}
	}
	else if (properties.indexed_pixels_)
	{
		detail::RendererUtils::indexed_to_rgba_pot(
			properties.width_,
			properties.height_,
			properties.actual_width_,
			properties.actual_height_,
			properties.indexed_is_column_major_,
			properties.indexed_pixels_,
			*properties.indexed_palette_,
			properties.indexed_alphas_,
			mipmap_buffer_
		);
	}
	else if (properties.indexed_sprite_)
	{
		detail::RendererUtils::indexed_sprite_to_rgba_pot(
			*properties.indexed_sprite_,
			*properties.indexed_palette_,
			mipmap_buffer_
		);
	}

	auto mipmap_width = properties.actual_width_;
	auto mipmap_height = properties.actual_height_;

	auto mipmap_count = properties.mipmap_count_;

	if (properties.is_generate_mipmaps_ &&
		properties.mipmap_count_ > 1 &&
		device_features.mipmap_is_available_)
	{
		mipmap_count = 1;
	}

	for (int i_mipmap = 0; i_mipmap < mipmap_count; ++i_mipmap)
	{
		if (i_mipmap > 0)
		{
			detail::RendererUtils::build_mipmap(
				mipmap_width,
				mipmap_height,
				texture_subbuffer_0,
				texture_subbuffer_1);

			if (mipmap_width > 1)
			{
				mipmap_width /= 2;
			}

			if (mipmap_height > 1)
			{
				mipmap_height /= 2;
			}

			if (is_set_subbuffer_0)
			{
				is_set_subbuffer_0 = false;

				texture_subbuffer_0 = &mipmap_buffer_[0];
			}

			std::swap(texture_subbuffer_0, texture_subbuffer_1);
		}

		auto param = RendererTexture2dUpdateParam{};
		param.mipmap_level_ = i_mipmap;
		param.rgba_pixels_ = texture_subbuffer_0;

		texture_2d->update(param);
	}

	if (properties.is_generate_mipmaps_ &&
		properties.mipmap_count_ > 1 &&
		device_features.mipmap_is_available_)
	{
		texture_2d->generate_mipmaps();
	}
}

void GenericHwTextureManager::destroy_missing_sprite_texture()
{
	if (missing_sprite_texture_2d_item_.texture_2d_ == nullptr)
	{
		return;
	}

	renderer_->texture_2d_destroy(missing_sprite_texture_2d_item_.texture_2d_);
	missing_sprite_texture_2d_item_.texture_2d_ = nullptr;
}

void GenericHwTextureManager::create_missing_sprite_texture()
{
	destroy_missing_sprite_texture();

	const auto& raw_image = get_missing_sprite_image();
	const auto rgba_image = reinterpret_cast<const R8g8b8a8*>(raw_image.data());

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = RendererPixelFormat::r8g8b8a8_unorm;
	param.width_ = Sprite::dimension;
	param.height_ = Sprite::dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::RendererUtils::calculate_mipmap_count(Sprite::dimension, Sprite::dimension);
	param.rgba_pixels_ = rgba_image;

	auto texture_2d_item = create_texture(param);

	missing_sprite_texture_2d_item_.properties_ = texture_2d_item.properties_;
	missing_sprite_texture_2d_item_.texture_2d_ = texture_2d_item.texture_2d_;

	update_mipmaps(missing_sprite_texture_2d_item_.properties_, missing_sprite_texture_2d_item_.texture_2d_);
}

void GenericHwTextureManager::destroy_missing_wall_texture()
{
	if (missing_wall_texture_2d_item_.texture_2d_ == nullptr)
	{
		return;
	}

	renderer_->texture_2d_destroy(missing_wall_texture_2d_item_.texture_2d_);
	missing_wall_texture_2d_item_.texture_2d_ = nullptr;
}

void GenericHwTextureManager::create_missing_wall_texture()
{
	destroy_missing_wall_texture();

	const auto& raw_image = get_missing_wall_image();
	const auto rgba_image = reinterpret_cast<const R8g8b8a8*>(raw_image.data());

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = RendererPixelFormat::r8g8b8_unorm;
	param.width_ = wall_dimension;
	param.height_ = wall_dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::RendererUtils::calculate_mipmap_count(param.width_, param.height_);
	param.rgba_pixels_ = rgba_image;

	auto texture_2d_item = create_texture(param);

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	missing_wall_texture_2d_item_ = texture_2d_item;
}

GenericHwTextureManager::Texture2dItem GenericHwTextureManager::wall_create_texture(
	const int wall_id)
{
	const auto indexed_pixels = static_cast<const std::uint8_t*>(::PM_GetPage(wall_id));

	if (indexed_pixels == nullptr)
	{
		throw Exception{"Null data."};
	}

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = RendererPixelFormat::r8g8b8_unorm;
	param.width_ = wall_dimension;
	param.height_ = wall_dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::RendererUtils::calculate_mipmap_count(param.width_, param.height_);
	param.indexed_is_column_major_ = true;
	param.indexed_pixels_ = indexed_pixels;
	param.indexed_palette_ = &::vid_hw_get_default_palette();

	auto texture_2d_item = create_texture(param);

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	return texture_2d_item;
}

GenericHwTextureManager::Texture2dItem GenericHwTextureManager::sprite_create_texture(
	const int sprite_id)
{
	auto sprite = sprite_cache_->cache(sprite_id);

	if (sprite == nullptr)
	{
		throw Exception{"Failed to cache a sprite #" + std::to_string(sprite_id) + "."};
	}

	if (!sprite->is_initialized())
	{
		throw Exception{"Sprite #" + std::to_string(sprite_id) + " not initialized."};
	}

	auto param = Texture2dProperties{};
	param.image_pixel_format_ = RendererPixelFormat::r8g8b8a8_unorm;
	param.width_ = Sprite::dimension;
	param.height_ = Sprite::dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::RendererUtils::calculate_mipmap_count(param.width_, param.height_);
	param.indexed_sprite_ = sprite;
	param.indexed_palette_ = &::vid_hw_get_default_palette();

	auto texture_2d_item = create_texture(param);

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	return texture_2d_item;
}

void GenericHwTextureManager::initialize_internal(
	RendererPtr renderer,
	SpriteCachePtr sprite_cache)
{
	if (renderer == nullptr)
	{
		throw Exception{"Null renderer."};
	}

	if (sprite_cache == nullptr)
	{
		throw Exception{"Null sprite cache."};
	}

	renderer_ = renderer;
	sprite_cache_ = sprite_cache;

	create_missing_sprite_texture();
	create_missing_wall_texture();

	generation_id_ = first_generation_id;

	wall_map_.reserve(max_walls);
	sprite_map_.reserve(max_sprites);

	upscale_filter_factor_ = upscale_filter_clamp_factor(upscale_filter_kind_, upscale_filter_factor_);
}

void GenericHwTextureManager::cache_purge(
	IdToTexture2dMap& map)
{
	for (auto map_it = map.begin(); map_it != map.end(); )
	{
		auto& texture_2d_item = map_it->second;

		if (texture_2d_item.generation_id_ != generation_id_)
		{
			const auto texture_2d = texture_2d_item.texture_2d_;
			texture_2d_item.texture_2d_ = nullptr;

			renderer_->texture_2d_destroy(texture_2d);

			map_it = map.erase(map_it);
		}
		else
		{
			++map_it;
		}
	}
}

RendererTexture2dPtr GenericHwTextureManager::get_texture_2d(
	const ImageKind image_kind,
	const int id,
	const IdToTexture2dMap& map) const
{
	auto item_it = map.find(id);

	if (item_it == map.end())
	{
		switch (image_kind)
		{
		case ImageKind::sprite:
			return missing_sprite_texture_2d_item_.texture_2d_;

		case ImageKind::wall:
			return missing_wall_texture_2d_item_.texture_2d_;

		default:
			return nullptr;
		}
	}

	return item_it->second.texture_2d_;
}

void GenericHwTextureManager::solid_1x1_destroy_all()
{
	for (int i = 0; i < static_cast<int>(HwTextureManagerSolid1x1Id::count_); ++i)
	{
		const auto id = static_cast<HwTextureManagerSolid1x1Id>(i);

		solid_1x1_destroy(id);
	}
}

int GenericHwTextureManager::solid_1x1_get_index(
	const HwTextureManagerSolid1x1Id id)
{
	switch (id)
	{
		case HwTextureManagerSolid1x1Id::black:
		case HwTextureManagerSolid1x1Id::white:
		case HwTextureManagerSolid1x1Id::fade_2d:
		case HwTextureManagerSolid1x1Id::fade_3d:
		case HwTextureManagerSolid1x1Id::flooring:
		case HwTextureManagerSolid1x1Id::ceiling:
			return static_cast<int>(id);

		default:
			throw Exception{"Unsupported solid 1x1 texture kind."};
	}
}

int GenericHwTextureManager::solid_1x1_get_updateable_index(
	const HwTextureManagerSolid1x1Id id)
{
	switch (id)
	{
		case HwTextureManagerSolid1x1Id::fade_2d:
		case HwTextureManagerSolid1x1Id::fade_3d:
		case HwTextureManagerSolid1x1Id::flooring:
		case HwTextureManagerSolid1x1Id::ceiling:
			return static_cast<int>(id);

		default:
			throw Exception{"Unsupported solid 1x1 texture kind."};
	}
}

R8g8b8a8 GenericHwTextureManager::solid_1x1_get_default_color(
	const HwTextureManagerSolid1x1Id id)
{
	switch (id)
	{
		case HwTextureManagerSolid1x1Id::black:
			return R8g8b8a8{0x00, 0x00, 0x00, 0xFF};

		case HwTextureManagerSolid1x1Id::white:
			return R8g8b8a8{0xFF, 0xFF, 0xFF, 0xFF};

		case HwTextureManagerSolid1x1Id::fade_2d:
		case HwTextureManagerSolid1x1Id::fade_3d:
			return R8g8b8a8{};

		case HwTextureManagerSolid1x1Id::flooring:
		case HwTextureManagerSolid1x1Id::ceiling:
			return R8g8b8a8{0x00, 0x00, 0x00, 0xFF};

		default:
			return R8g8b8a8{};
	}
}

int GenericHwTextureManager::upscale_filter_get_min_factor_internal(
	const HwTextureManagerUpscaleFilterKind upscale_filter_kind)
{
	switch (upscale_filter_kind)
	{
		case HwTextureManagerUpscaleFilterKind::none:
			return 1;

		case HwTextureManagerUpscaleFilterKind::xbrz:
			return 2;

		default:
			throw Exception{"Unsupported upscale filter kind."};
	}
}

int GenericHwTextureManager::upscale_filter_get_max_factor_internal(
	const HwTextureManagerUpscaleFilterKind upscale_filter_kind)
{
	switch (upscale_filter_kind)
	{
		case HwTextureManagerUpscaleFilterKind::none:
			return 1;

		case HwTextureManagerUpscaleFilterKind::xbrz:
			return xbrz::SCALE_FACTOR_MAX;

		default:
			throw Exception{"Unsupported upscale filter kind."};
	}
}

int GenericHwTextureManager::upscale_filter_clamp_factor(
	const HwTextureManagerUpscaleFilterKind upscale_filter_kind,
	const int upscale_filter_factor)
{
	auto result = upscale_filter_factor;


	const auto min_result = upscale_filter_get_min_factor_internal(upscale_filter_kind);

	if (result < min_result)
	{
		result = min_result;
	}


	const auto max_result = upscale_filter_get_max_factor_internal(upscale_filter_kind);

	if (result > max_result)
	{
		result = max_result;
	}

	return result;
}

//
// GenericHwTextureManager
// ==========================================================================


// ==========================================================================
// HwTextureManagerFactory
//

HwTextureManagerUPtr HwTextureManagerFactory::create(
	RendererPtr renderer,
	SpriteCachePtr sprite_cache)
{
	return HwTextureManagerImplUPtr{new GenericHwTextureManager{renderer, sprite_cache}};
}

//
// HwTextureManagerFactory
// ==========================================================================


} // bstone
