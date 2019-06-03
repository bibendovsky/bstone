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
#include <cassert>
#include <array>
#include <unordered_map>
#include "id_pm.h"
#include "id_vl.h"
#include "bstone_hw_texture_manager.h"
#include "bstone_missing_sprite_64x64_image.h"
#include "bstone_missing_wall_64x64_image.h"
#include "bstone_ref_values.h"
#include "bstone_detail_renderer_utils.h"
#include "bstone_sprite_cache.h"


namespace bstone
{


// ==========================================================================
// RendererTextureManagerImpl
//

class RendererTextureManagerImpl :
	public HwTextureManager
{
public:
	RendererTextureManagerImpl() = default;

	~RendererTextureManagerImpl() override = default;


	bool is_initialized() const override;

	const std::string& get_error_message() const override;

	void uninitialize() override;


	bool set_palette(
		const RendererPalette& palette) override;


	bool cache_begin() override;

	bool cache_end() override;

	bool cache_purge() override;


	bool wall_cache(
		const int id) override;

	RendererTexture2dPtr wall_get(
		const int id) const override;


	bool sprite_cache(
		const int id) override;

	RendererTexture2dPtr sprite_get(
		const int id) const override;


	void ui_destroy() override;

	bool ui_create(
		const std::uint8_t* const indexed_pixels,
		const bool* const indexed_alphas,
		const RendererPaletteCPtr indexed_palette) override;

	void ui_update() override;

	RendererTexture2dPtr ui_get() const override;


	void solid_1x1_destroy(
		const HwTextureManagerSolid1x1Id id) override;

	bool solid_1x1_create(
		const HwTextureManagerSolid1x1Id id) override;

	void solid_1x1_update(
		const HwTextureManagerSolid1x1Id id,
		const RendererColor32 color) override;

	RendererTexture2dPtr solid_1x1_get(
		const HwTextureManagerSolid1x1Id id) const override;


	bool device_on_reset() override;


	void initialize(
		RendererPtr renderer,
		SpriteCachePtr sprite_cache);


private:
	class Detail;


	static Detail& get_instance();
}; // RendererTextureManagerImpl

using RendererTextureManagerImplUPtr = std::unique_ptr<RendererTextureManagerImpl>;

//
// RendererTextureManagerImpl
// ==========================================================================


// ==========================================================================
// RendererTextureManagerImpl::Detail
//

class RendererTextureManagerImpl::Detail
{
public:
	Detail();

	Detail(
		const Detail& rhs) = delete;

	Detail(
		Detail&& rhs);

	~Detail();


	bool is_initialized() const;

	const std::string& get_error_message() const;

	void uninitialize();


	bool set_palette(
		const RendererPalette& palette);


	bool cache_begin();

	bool cache_end();

	bool cache_purge();


	bool wall_cache(
		const int id);

	RendererTexture2dPtr wall_get(
		const int id) const;


	bool sprite_cache(
		const int id);

	RendererTexture2dPtr sprite_get(
		const int id) const;


	void ui_destroy();

	bool ui_create(
		const std::uint8_t* const indexed_pixels,
		const bool* const indexed_alphas,
		const RendererPaletteCPtr indexed_palette);

	void ui_update();

	RendererTexture2dPtr ui_get() const;


	void solid_1x1_destroy(
		const HwTextureManagerSolid1x1Id id);

	bool solid_1x1_create(
		const HwTextureManagerSolid1x1Id id);

	void solid_1x1_update(
		const HwTextureManagerSolid1x1Id id,
		const RendererColor32 color);

	RendererTexture2dPtr solid_1x1_get(
		const HwTextureManagerSolid1x1Id id) const;


	bool device_on_reset();


	void initialize(
		RendererPtr renderer,
		SpriteCachePtr sprite_cache);

	void uninitialize_internal();


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
		bool is_npot_;

		int width_;
		int height_;

		int actual_width_;
		int actual_height_;

		bool is_generate_mipmaps_;
		int mipmap_count_;

		bool indexed_is_column_major_;
		const std::uint8_t* indexed_pixels_;
		const RendererPalette* indexed_palette_;
		const bool* indexed_alphas_;

		SpriteCPtr indexed_sprite_;

		RendererColor32CPtr rgba_pixels_;
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
		RendererColor32 color_;
		Texture2dProperties properties_;
		RendererTexture2dPtr texture_2d_;
	}; // Solid1x1Item

	using Solid1x1Items = std::array<Solid1x1Item, static_cast<std::size_t>(HwTextureManagerSolid1x1Id::count_)>;



	bool is_initialized_;
	mutable std::string error_message_;

	RendererPtr renderer_;
	SpriteCachePtr sprite_cache_;

	bool is_caching_;
	GenerationId generation_id_;

	IdToTexture2dMap wall_map_;
	IdToTexture2dMap sprite_map_;

	Texture2dItem missing_sprite_texture_2d_item_;
	Texture2dItem missing_wall_texture_2d_item_;

	RendererPalette palette_;

	Texture2dItem ui_t2d_item_;

	detail::RendererUtils::TextureBuffer texture_buffer_;

	Solid1x1Items solid_1x1_items_;


	bool validate_image_source_texture_2d_properties(
		const Texture2dProperties& properties);

	bool validate_dimensions_texture_2d_properties(
		const Texture2dProperties& properties);

	bool validate_mipmap_texture_2d_properties(
		const Texture2dProperties& properties);

	bool validate_common_texture_2d_properties(
		const Texture2dProperties& properties);

	bool validate_indexed_texture_2d_properties(
		const Texture2dProperties& properties);

	bool validate_indexed_sprite_texture_2d_properties(
		const Texture2dProperties& properties);

	bool validate_rgba_texture_2d_properties(
		const Texture2dProperties& properties);

	bool validate_source_texture_2d_properties(
		const Texture2dProperties& properties);

	bool validate_texture_2d_properties(
		const Texture2dProperties& properties);

	Texture2dItem create_texture(
		const Texture2dProperties& properties);

	void update_mipmaps(
		const Texture2dProperties& properties,
		RendererTexture2dPtr texture_2d);


	void destroy_missing_sprite_texture();

	bool create_missing_sprite_texture();


	void destroy_missing_wall_texture();

	bool create_missing_wall_texture();


	Texture2dItem wall_create_texture(
		const int wall_id);

	Texture2dItem sprite_create_texture(
		const int sprite_id);


	bool initialize_internal(
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

	static RendererColor32 solid_1x1_get_default_color(
		const HwTextureManagerSolid1x1Id id);
}; // Detail


RendererTextureManagerImpl::Detail::Detail()
	:
	is_initialized_{},
	error_message_{},
	renderer_{},
	sprite_cache_{},
	is_caching_{},
	generation_id_{},
	wall_map_{},
	sprite_map_{},
	missing_sprite_texture_2d_item_{},
	missing_wall_texture_2d_item_{},
	palette_{},
	ui_t2d_item_{},
	texture_buffer_{},
	solid_1x1_items_{}
{
}

RendererTextureManagerImpl::Detail::Detail(
	Detail&& rhs)
	:
	is_initialized_{std::move(rhs.is_initialized_)},
	error_message_{std::move(rhs.error_message_)},
	renderer_{std::move(rhs.renderer_)},
	sprite_cache_{std::move(rhs.sprite_cache_)},
	is_caching_{std::move(rhs.is_caching_)},
	generation_id_{std::move(rhs.generation_id_)},
	wall_map_{std::move(rhs.wall_map_)},
	sprite_map_{std::move(rhs.sprite_map_)},
	missing_sprite_texture_2d_item_{std::move(rhs.missing_sprite_texture_2d_item_)},
	missing_wall_texture_2d_item_{std::move(rhs.missing_wall_texture_2d_item_)},
	palette_{std::move(rhs.palette_)},
	ui_t2d_item_{std::move(rhs.ui_t2d_item_)},
	texture_buffer_{std::move(rhs.texture_buffer_)},
	solid_1x1_items_{std::move(rhs.solid_1x1_items_)}
{
	rhs.is_initialized_ = false;
	rhs.missing_sprite_texture_2d_item_.texture_2d_ = nullptr;
	rhs.missing_wall_texture_2d_item_.texture_2d_ = nullptr;
	rhs.ui_t2d_item_.texture_2d_ = nullptr;
	rhs.solid_1x1_items_.fill(Solid1x1Item{});
}

RendererTextureManagerImpl::Detail::~Detail()
{
	uninitialize_internal();
}

bool RendererTextureManagerImpl::Detail::is_initialized() const
{
	return is_initialized_;
}

const std::string& RendererTextureManagerImpl::Detail::get_error_message() const
{
	return error_message_;
}

void RendererTextureManagerImpl::Detail::uninitialize()
{
	uninitialize_internal();
}

bool RendererTextureManagerImpl::Detail::set_palette(
	const RendererPalette& palette)
{
	if (!is_initialized_)
	{
		error_message_ = "Not initialized.";

		return false;
	}

	palette_ = palette;

	// TODO

	return true;
}

bool RendererTextureManagerImpl::Detail::cache_begin()
{
	if (!is_initialized_)
	{
		error_message_ = "Not initialized.";

		return false;
	}

	if (is_caching_)
	{
		error_message_ = "Already caching.";

		return false;
	}

	is_caching_ = true;

	++generation_id_;

	if (generation_id_ == invalid_generation_id)
	{
		generation_id_ = first_generation_id;
	}

	return true;
}

bool RendererTextureManagerImpl::Detail::cache_end()
{
	if (!is_initialized_)
	{
		error_message_ = "Not initialized.";

		return false;
	}

	if (!is_caching_)
	{
		error_message_ = "Not caching.";

		return false;
	}

	is_caching_ = false;

	return true;
}

bool RendererTextureManagerImpl::Detail::cache_purge()
{
	if (!is_initialized_)
	{
		error_message_ = "Not initialized.";

		return false;
	}

	if (is_caching_)
	{
		error_message_ = "Caching is active.";

		return false;
	}

	cache_purge(wall_map_);
	cache_purge(sprite_map_);

	return true;
}

bool RendererTextureManagerImpl::Detail::wall_cache(
	const int id)
{
	if (!is_initialized_)
	{
		error_message_ = "Not initialized.";

		return false;
	}

	if (id < 0 || id >= max_walls)
	{
		error_message_ = "Id out of range.";

		return false;
	}

	auto wall_it = wall_map_.find(id);

	if (wall_it != wall_map_.end())
	{
		wall_it->second.generation_id_ = generation_id_;

		return true;
	}

	auto texture_2d_item = wall_create_texture(id);

	if (texture_2d_item.texture_2d_ == nullptr)
	{
		return false;
	}

	texture_2d_item.generation_id_ = generation_id_;

	wall_map_[id] = texture_2d_item;

	return true;
}

RendererTexture2dPtr RendererTextureManagerImpl::Detail::wall_get(
	const int id) const
{
	if (!is_initialized_)
	{
		error_message_ = "Not initialized.";

		return nullptr;
	}

	if (id < 0 || id >= max_walls)
	{
		error_message_ = "Id out of range.";

		return nullptr;
	}

	return get_texture_2d(ImageKind::wall, id, wall_map_);
}

bool RendererTextureManagerImpl::Detail::sprite_cache(
	const int id)
{
	if (!is_initialized_)
	{
		error_message_ = "Not initialized.";

		return false;
	}

	if (id <= 0 || id >= max_sprites)
	{
		error_message_ = "Id out of range.";

		return false;
	}

	auto sprite_it = sprite_map_.find(id);

	if (sprite_it != sprite_map_.end())
	{
		sprite_it->second.generation_id_ = generation_id_;

		return true;
	}

	auto texture_2d_item = sprite_create_texture(id);

	if (texture_2d_item.texture_2d_ == nullptr)
	{
		return false;
	}

	texture_2d_item.generation_id_ = generation_id_;

	sprite_map_[id] = texture_2d_item;

	return true;
}

RendererTexture2dPtr RendererTextureManagerImpl::Detail::sprite_get(
	const int id) const
{
	if (!is_initialized_)
	{
		error_message_ = "Not initialized.";

		return nullptr;
	}

	if (id <= 0 || id >= max_sprites)
	{
		error_message_ = "Id out of range.";

		return nullptr;
	}

	return get_texture_2d(ImageKind::sprite, id, sprite_map_);
}

void RendererTextureManagerImpl::Detail::ui_destroy()
{
	if (ui_t2d_item_.texture_2d_ == nullptr)
	{
		return;
	}

	renderer_->texture_2d_destroy(ui_t2d_item_.texture_2d_);
	ui_t2d_item_.texture_2d_ = nullptr;
}

bool RendererTextureManagerImpl::Detail::ui_create(
	const std::uint8_t* const indexed_pixels,
	const bool* const indexed_alphas,
	const RendererPaletteCPtr indexed_palette)
{
	if (ui_t2d_item_.texture_2d_ != nullptr)
	{
		error_message_ = "Already created.";

		return false;
	}

	if (indexed_pixels == nullptr)
	{
		error_message_ = "Null indexed pixels.";

		return false;
	}

	if (indexed_alphas == nullptr)
	{
		error_message_ = "Null indexed alphas.";

		return false;
	}

	if (indexed_palette == nullptr)
	{
		error_message_ = "Null indexed palette.";

		return false;
	}

	auto param = Texture2dProperties{};
	param.width_ = ::vga_ref_width;
	param.height_ = ::vga_ref_height;
	param.mipmap_count_ = 1;
	param.indexed_pixels_ = indexed_pixels;
	param.indexed_palette_ = indexed_palette;
	param.indexed_alphas_ = indexed_alphas;

	auto texture_2d_item = create_texture(param);

	if (texture_2d_item.texture_2d_ == nullptr)
	{
		error_message_ = "Failed to create UI texture. " + error_message_;

		return false;
	}

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	ui_t2d_item_ = texture_2d_item;

	return true;
}

void RendererTextureManagerImpl::Detail::ui_update()
{
	if (ui_t2d_item_.texture_2d_ == nullptr)
	{
		assert(!"Not created.");

		return;
	}

	update_mipmaps(ui_t2d_item_.properties_, ui_t2d_item_.texture_2d_);
}

RendererTexture2dPtr RendererTextureManagerImpl::Detail::ui_get() const
{
	assert(ui_t2d_item_.texture_2d_ != nullptr);

	return ui_t2d_item_.texture_2d_;
}

void RendererTextureManagerImpl::Detail::solid_1x1_destroy(
	const HwTextureManagerSolid1x1Id id)
{
	const auto index = solid_1x1_get_index(id);

	if (index < 0)
	{
		assert(!"Invalid index.");

		return;
	}

	auto& item = solid_1x1_items_[index];

	if (item.texture_2d_ == nullptr)
	{
		return;
	}

	item.color_ = {};

	renderer_->texture_2d_destroy(item.texture_2d_);
	item.texture_2d_ = nullptr;
}

bool RendererTextureManagerImpl::Detail::solid_1x1_create(
	const HwTextureManagerSolid1x1Id id)
{
	const auto index = solid_1x1_get_index(id);

	if (index < 0)
	{
		error_message_ = "Invalid index.";

		return false;
	}

	const auto default_color = solid_1x1_get_default_color(id);
	const auto has_alpha = (default_color.a < 0xFF);

	const auto internal_format = (has_alpha ? bstone::RendererPixelFormat::r8g8b8a8_unorm : bstone::RendererPixelFormat::r8g8b8_unorm);

	auto param = Texture2dProperties{};
	param.width_ = 1;
	param.height_ = 1;
	param.mipmap_count_ = 1;
	param.rgba_pixels_ = &default_color;

	auto texture_2d_item = create_texture(param);

	if (texture_2d_item.texture_2d_ == nullptr)
	{
		error_message_ = "Failed to create solid 1x1 texture. " + error_message_;

		return false;
	}

	auto& item = solid_1x1_items_[index];
	item.color_ = default_color;
	item.properties_ = texture_2d_item.properties_;
	item.texture_2d_ = item.texture_2d_;

	update_mipmaps(item.properties_, item.texture_2d_);

	return true;
}

void RendererTextureManagerImpl::Detail::solid_1x1_update(
	const HwTextureManagerSolid1x1Id id,
	const RendererColor32 color)
{
	const auto index = solid_1x1_get_updateable_index(id);

	if (index < 0)
	{
		assert(!"Non-updateable solid 1x1 texture.");

		return;
	}

	auto& item = solid_1x1_items_[index];
	item.color_ = color;

	auto param = RendererTexture2dUpdateParam{};
	param.rgba_pixels_ = &item.color_;

	item.texture_2d_->update(param);
}

RendererTexture2dPtr RendererTextureManagerImpl::Detail::solid_1x1_get(
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

bool RendererTextureManagerImpl::Detail::device_on_reset()
{
	if (is_initialized_)
	{
		error_message_ = "Not initialized.";

		return false;
	}

	// Missing sprite texture.
	//
	{
		destroy_missing_sprite_texture();

		if (!create_missing_sprite_texture())
		{
			return false;
		}
	}

	// Missing wall texture.
	//
	{
		destroy_missing_wall_texture();

		if (!create_missing_wall_texture())
		{
			return false;
		}
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

		if (texture_2d_item.texture_2d_ == nullptr)
		{
			return false;
		}

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

		if (texture_2d_item.texture_2d_ == nullptr)
		{
			return false;
		}

		texture_2d_item.generation_id_ = generation_id_;
	}

	// UI texture.
	//
	{
		ui_destroy();

		if (!ui_create(
			ui_t2d_item_.properties_.indexed_pixels_,
			ui_t2d_item_.properties_.indexed_alphas_,
			ui_t2d_item_.properties_.indexed_palette_))
		{
			return false;
		}
	}

	// Solid 1x1 textures.
	//
	for (int i = 0; i < static_cast<int>(HwTextureManagerSolid1x1Id::count_); ++i)
	{
		const auto id = static_cast<HwTextureManagerSolid1x1Id>(i);

		solid_1x1_destroy(id);

		if (!solid_1x1_create(id))
		{
			return false;
		}
	}


	return true;
}

void RendererTextureManagerImpl::Detail::initialize(
	RendererPtr renderer,
	SpriteCachePtr sprite_cache)
{
	if (is_initialized_)
	{
		return;
	}

	if (!initialize_internal(renderer, sprite_cache))
	{
		uninitialize_internal();

		return;
	}

	is_initialized_ = true;
}

void RendererTextureManagerImpl::Detail::uninitialize_internal()
{
	is_initialized_ = false;
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
	texture_buffer_.clear();
}

bool RendererTextureManagerImpl::Detail::validate_image_source_texture_2d_properties(
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
		error_message_ = "No image source.";

		return false;
	}

	if (source_count > 1)
	{
		error_message_ = "Multiple image source.";

		return false;
	}

	return true;
}

bool RendererTextureManagerImpl::Detail::validate_dimensions_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.width_ <= 0)
	{
		error_message_ = "Invalid width.";

		return false;
	}

	if (properties.height_ <= 0)
	{
		error_message_ = "Invalid height.";

		return false;
	}

	return true;
}

bool RendererTextureManagerImpl::Detail::validate_mipmap_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.mipmap_count_ <= 0 ||
		properties.mipmap_count_ > detail::RendererUtils::get_max_mipmap_count())
	{
		error_message_ = "Mipmap count out of range.";

		return false;
	}

	return true;
}

bool RendererTextureManagerImpl::Detail::validate_common_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (!validate_image_source_texture_2d_properties(properties))
	{
		return false;
	}

	if (!validate_dimensions_texture_2d_properties(properties))
	{
		return false;
	}

	if (!validate_mipmap_texture_2d_properties(properties))
	{
		return false;
	}

	return true;
}

bool RendererTextureManagerImpl::Detail::validate_indexed_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.indexed_pixels_ == nullptr)
	{
		error_message_ = "Null indexed image source.";

		return false;
	}

	if (properties.indexed_palette_ == nullptr)
	{
		error_message_ = "Null indexed palette.";

		return false;
	}

	return true;
}

bool RendererTextureManagerImpl::Detail::validate_indexed_sprite_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.indexed_sprite_ == nullptr)
	{
		error_message_ = "Null indexed sprite.";

		return false;
	}

	if (properties.indexed_palette_ == nullptr)
	{
		error_message_ = "Null indexed palette.";

		return false;
	}

	return true;
}

bool RendererTextureManagerImpl::Detail::validate_rgba_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.rgba_pixels_ == nullptr)
	{
		error_message_ = "Null RGBA image.";

		return false;
	}

	return true;
}

bool RendererTextureManagerImpl::Detail::validate_source_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (properties.indexed_pixels_ != nullptr)
	{
		return validate_indexed_texture_2d_properties(properties);
	}
	else if (properties.indexed_sprite_ != nullptr)
	{
		return validate_indexed_sprite_texture_2d_properties(properties);
	}
	else if (properties.rgba_pixels_ != nullptr)
	{
		return validate_rgba_texture_2d_properties(properties);
	}
	else
	{
		error_message_ = "No image source.";

		return false;
	}
}

bool RendererTextureManagerImpl::Detail::validate_texture_2d_properties(
	const Texture2dProperties& properties)
{
	if (!validate_common_texture_2d_properties(properties))
	{
		return false;
	}

	if (!validate_source_texture_2d_properties(properties))
	{
		return false;
	}

	return true;
}

RendererTextureManagerImpl::Detail::Texture2dItem RendererTextureManagerImpl::Detail::create_texture(
	const Texture2dProperties& properties)
{
	if (!validate_texture_2d_properties(properties))
	{
		return Texture2dItem{};
	}

	auto renderer_utils = detail::RendererUtils{};

	const auto& device_features = renderer_->device_get_features();

	auto new_properties = properties;

	// Calculate actual dimensions.
	//
	new_properties.actual_width_ = renderer_utils.find_nearest_pot_value(properties.width_);

	if (new_properties.actual_width_ > device_features.max_texture_dimension_)
	{
		new_properties.actual_width_ = device_features.max_texture_dimension_;
	}

	new_properties.actual_height_ = renderer_utils.find_nearest_pot_value(properties.height_);

	if (new_properties.actual_height_ > device_features.max_texture_dimension_)
	{
		new_properties.actual_height_ = device_features.max_texture_dimension_;
	}

	// Check for non-power-of-two dimensions.
	//
	new_properties.is_npot_ = (
		new_properties.width_ != new_properties.actual_width_ ||
		new_properties.height_ != new_properties.actual_height_);

	if (new_properties.is_npot_)
	{
		if (device_features.npot_is_available_)
		{
			new_properties.actual_width_ = new_properties.width_;
			new_properties.actual_height_ = new_properties.height_;
		}
	}

	// Create texture object.
	//
	auto texture_2d = RendererTexture2dPtr{};
	// FIXME

	// Return the result.
	//
	auto result = Texture2dItem{};

	result.properties_ = new_properties;
	result.texture_2d_ = texture_2d;

	return result;
}

void RendererTextureManagerImpl::Detail::update_mipmaps(
	const Texture2dProperties& properties,
	RendererTexture2dPtr texture_2d)
{
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

	if (static_cast<int>(texture_buffer_.size()) < max_buffer_size)
	{
		texture_buffer_.resize(max_buffer_size);
	}

	auto texture_subbuffer_0 = &texture_buffer_[0];
	auto texture_subbuffer_1 = RendererColor32Ptr{};

	if (is_manual_mipmaps)
	{
		texture_subbuffer_1 = &texture_buffer_[max_subbuffer_size];
	}

	auto is_set_subbuffer_0 = false;

	if (properties.rgba_pixels_ != nullptr)
	{
		if (properties.is_npot_ && !npot_is_available)
		{
			detail::RendererUtils::rgba_npot_to_rgba_pot(
				properties.width_,
				properties.height_,
				properties.actual_width_,
				properties.actual_height_,
				properties.rgba_pixels_,
				texture_buffer_
			);
		}
		else
		{
			// Don't copy the base mipmap into a buffer.

			is_set_subbuffer_0 = true;

			texture_subbuffer_0 = const_cast<RendererColor32Ptr>(properties.rgba_pixels_);
		}
	}
	else if (properties.indexed_pixels_ != nullptr)
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
			texture_buffer_
		);
	}
	else if (properties.indexed_sprite_ != nullptr)
	{
		detail::RendererUtils::indexed_sprite_to_rgba_pot(
			*properties.indexed_sprite_,
			*properties.indexed_palette_,
			texture_buffer_
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

				texture_subbuffer_0 = &texture_buffer_[0];
			}

			std::swap(texture_subbuffer_0, texture_subbuffer_1);
		}

		//upload_mipmap(i_mipmap, mipmap_width, mipmap_height, texture_subbuffer_0);
	}

	if (properties.is_generate_mipmaps_ &&
		properties.mipmap_count_ > 1 &&
		device_features.mipmap_is_available_)
	{
	}
}

void RendererTextureManagerImpl::Detail::destroy_missing_sprite_texture()
{
	if (missing_sprite_texture_2d_item_.texture_2d_ == nullptr)
	{
		return;
	}

	renderer_->texture_2d_destroy(missing_sprite_texture_2d_item_.texture_2d_);
	missing_sprite_texture_2d_item_.texture_2d_ = nullptr;
}

bool RendererTextureManagerImpl::Detail::create_missing_sprite_texture()
{
	destroy_missing_sprite_texture();

	const auto& raw_image = get_missing_sprite_image();
	const auto rgba_image = reinterpret_cast<const RendererColor32*>(raw_image.data());

	auto param = Texture2dProperties{};
	param.width_ = Sprite::dimension;
	param.height_ = Sprite::dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::RendererUtils::calculate_mipmap_count(Sprite::dimension, Sprite::dimension);
	param.rgba_pixels_ = rgba_image;

	auto texture_2d_item = create_texture(param);

	if (texture_2d_item.texture_2d_ == nullptr)
	{
		error_message_ = "Failed to create a missing sprite texture. " + error_message_;

		return false;
	}

	missing_sprite_texture_2d_item_.properties_ = texture_2d_item.properties_;
	missing_sprite_texture_2d_item_.texture_2d_ = texture_2d_item.texture_2d_;

	update_mipmaps(missing_sprite_texture_2d_item_.properties_, missing_sprite_texture_2d_item_.texture_2d_);

	return true;
}

void RendererTextureManagerImpl::Detail::destroy_missing_wall_texture()
{
	if (missing_wall_texture_2d_item_.texture_2d_ == nullptr)
	{
		return;
	}

	renderer_->texture_2d_destroy(missing_wall_texture_2d_item_.texture_2d_);
	missing_wall_texture_2d_item_.texture_2d_ = nullptr;
}

bool RendererTextureManagerImpl::Detail::create_missing_wall_texture()
{
	destroy_missing_wall_texture();

	const auto& raw_image = get_missing_wall_image();
	const auto rgba_image = reinterpret_cast<const RendererColor32*>(raw_image.data());

	auto param = Texture2dProperties{};
	param.width_ = wall_dimension;
	param.height_ = wall_dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::RendererUtils::calculate_mipmap_count(param.width_, param.height_);
	param.rgba_pixels_ = rgba_image;

	auto texture_2d_item = create_texture(param);

	if (texture_2d_item.texture_2d_ == nullptr)
	{
		error_message_ = "Failed to create a missing wall texture. " + error_message_;

		return false;
	}

	// TODO
	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	missing_wall_texture_2d_item_ = texture_2d_item;

	return true;
}

RendererTextureManagerImpl::Detail::Texture2dItem RendererTextureManagerImpl::Detail::wall_create_texture(
	const int wall_id)
{
	const auto indexed_pixels = static_cast<const std::uint8_t*>(::PM_GetPage(wall_id));

	if (indexed_pixels == nullptr)
	{
		error_message_ = "Null data.";

		return Texture2dItem{};
	}

	auto param = Texture2dProperties{};
	param.width_ = wall_dimension;
	param.height_ = wall_dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::RendererUtils::calculate_mipmap_count(param.width_, param.height_);
	param.indexed_is_column_major_ = true;
	param.indexed_pixels_ = indexed_pixels;
	param.indexed_palette_ = &::vid_hw_get_default_palette();

	auto texture_2d_item = create_texture(param);

	if (texture_2d_item.texture_2d_ == nullptr)
	{
		error_message_ = "Failed to create a wall texture. " + error_message_;

		return Texture2dItem{};
	}

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	return texture_2d_item;
}

RendererTextureManagerImpl::Detail::Texture2dItem RendererTextureManagerImpl::Detail::sprite_create_texture(
	const int sprite_id)
{
	auto sprite = sprite_cache_->cache(sprite_id);

	if (sprite == nullptr)
	{
		error_message_ = "Failed to cache a sprite #" + std::to_string(sprite_id) + ".";

		return Texture2dItem{};
	}

	if (!sprite->is_initialized())
	{
		error_message_ = "Sprite #" + std::to_string(sprite_id) + " not initialized.";

		return Texture2dItem{};
	}

	auto param = Texture2dProperties{};
	param.width_ = Sprite::dimension;
	param.height_ = Sprite::dimension;
	param.is_generate_mipmaps_ = true;
	param.mipmap_count_ = detail::RendererUtils::calculate_mipmap_count(param.width_, param.height_);
	param.indexed_sprite_ = sprite;
	param.indexed_palette_ = &::vid_hw_get_default_palette();

	auto texture_2d_item = create_texture(param);

	if (texture_2d_item.texture_2d_ == nullptr)
	{
		error_message_ = "Failed to create a sprite texture. " + error_message_;

		return Texture2dItem{};
	}

	update_mipmaps(texture_2d_item.properties_, texture_2d_item.texture_2d_);

	return texture_2d_item;
}

bool RendererTextureManagerImpl::Detail::initialize_internal(
	RendererPtr renderer,
	SpriteCachePtr sprite_cache)
{
	if (renderer == nullptr)
	{
		error_message_ = "Null renderer.";

		return false;
	}

	if (sprite_cache == nullptr)
	{
		error_message_ = "Null sprite cache.";

		return false;
	}

	renderer_ = renderer;
	sprite_cache_ = sprite_cache;

	if (!create_missing_sprite_texture())
	{
		return false;
	}

	if (!create_missing_wall_texture())
	{
		return false;
	}

	generation_id_ = first_generation_id;

	wall_map_.reserve(max_walls);
	sprite_map_.reserve(max_sprites);

	return true;
}

void RendererTextureManagerImpl::Detail::cache_purge(
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

RendererTexture2dPtr RendererTextureManagerImpl::Detail::get_texture_2d(
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

void RendererTextureManagerImpl::Detail::solid_1x1_destroy_all()
{
	for (int i = 0; i < static_cast<int>(HwTextureManagerSolid1x1Id::count_); ++i)
	{
		const auto id = static_cast<HwTextureManagerSolid1x1Id>(i);

		solid_1x1_destroy(id);
	}
}

int RendererTextureManagerImpl::Detail::solid_1x1_get_index(
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
			return -1;
	}
}

int RendererTextureManagerImpl::Detail::solid_1x1_get_updateable_index(
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
			return -1;
	}
}

RendererColor32 RendererTextureManagerImpl::Detail::solid_1x1_get_default_color(
	const HwTextureManagerSolid1x1Id id)
{
	switch (id)
	{
		case HwTextureManagerSolid1x1Id::black:
			return RendererColor32{0x00, 0x00, 0x00, 0xFF};

		case HwTextureManagerSolid1x1Id::white:
			return RendererColor32{0xFF, 0xFF, 0xFF, 0xFF};

		case HwTextureManagerSolid1x1Id::fade_2d:
		case HwTextureManagerSolid1x1Id::fade_3d:
			return RendererColor32{};

		case HwTextureManagerSolid1x1Id::flooring:
		case HwTextureManagerSolid1x1Id::ceiling:
			return RendererColor32{0x00, 0x00, 0x00, 0xFF};

		default:
			return RendererColor32{};
	}
}

//
// RendererTextureManagerImpl::Detail
// ==========================================================================


// ==========================================================================
// RendererTextureManagerImpl
//

bool RendererTextureManagerImpl::is_initialized() const
{
	auto& instance = get_instance();

	return instance.is_initialized();
}

const std::string& RendererTextureManagerImpl::get_error_message() const
{
	auto& instance = get_instance();

	return instance.get_error_message();
}

void RendererTextureManagerImpl::uninitialize()
{
	auto& instance = get_instance();

	instance.uninitialize();
}

bool RendererTextureManagerImpl::set_palette(
	const RendererPalette& palette)
{
	auto& instance = get_instance();

	return instance.set_palette(palette);
}

bool RendererTextureManagerImpl::cache_begin()
{
	auto& instance = get_instance();

	return instance.cache_begin();
}

bool RendererTextureManagerImpl::cache_end()
{
	auto& instance = get_instance();

	return instance.cache_end();
}

bool RendererTextureManagerImpl::cache_purge()
{
	auto& instance = get_instance();

	return instance.cache_purge();
}

bool RendererTextureManagerImpl::wall_cache(
	const int id)
{
	auto& instance = get_instance();

	return instance.wall_cache(id);
}

RendererTexture2dPtr RendererTextureManagerImpl::wall_get(
	const int id) const
{
	auto& instance = get_instance();

	return instance.wall_get(id);
}

bool RendererTextureManagerImpl::sprite_cache(
	const int id)
{
	auto& instance = get_instance();

	return instance.sprite_cache(id);
}

RendererTexture2dPtr RendererTextureManagerImpl::sprite_get(
	const int id) const
{
	auto& instance = get_instance();

	return instance.sprite_get(id);
}

void RendererTextureManagerImpl::ui_destroy()
{
	auto& instance = get_instance();

	instance.ui_destroy();
}

bool RendererTextureManagerImpl::ui_create(
	const std::uint8_t* const indexed_pixels,
	const bool* const indexed_alphas,
	const RendererPaletteCPtr indexed_palette)
{
	auto& instance = get_instance();

	return instance.ui_create(indexed_pixels, indexed_alphas, indexed_palette);
}

void RendererTextureManagerImpl::ui_update()
{
	auto& instance = get_instance();

	instance.ui_update();
}

RendererTexture2dPtr RendererTextureManagerImpl::ui_get() const
{
	auto& instance = get_instance();

	return instance.ui_get();
}

void RendererTextureManagerImpl::solid_1x1_destroy(
	const HwTextureManagerSolid1x1Id id)
{
	auto& instance = get_instance();

	instance.solid_1x1_destroy(id);
}

bool RendererTextureManagerImpl::solid_1x1_create(
	const HwTextureManagerSolid1x1Id id)
{
	auto& instance = get_instance();

	return instance.solid_1x1_create(id);
}

void RendererTextureManagerImpl::solid_1x1_update(
	const HwTextureManagerSolid1x1Id id,
	const RendererColor32 color)
{
	auto& instance = get_instance();

	instance.solid_1x1_update(id, color);
}

RendererTexture2dPtr RendererTextureManagerImpl::solid_1x1_get(
	const HwTextureManagerSolid1x1Id id) const
{
	auto& instance = get_instance();

	return instance.solid_1x1_get(id);
}

bool RendererTextureManagerImpl::device_on_reset()
{
	auto& instance = get_instance();

	return instance.device_on_reset();
}

void RendererTextureManagerImpl::initialize(
	RendererPtr renderer,
	SpriteCachePtr sprite_cache)
{
	auto& instance = get_instance();

	return instance.initialize(renderer, sprite_cache);
}

RendererTextureManagerImpl::Detail& RendererTextureManagerImpl::get_instance()
{
	static auto result = Detail{};

	return result;
}

//
// RendererTextureManagerImpl
// ==========================================================================


// ==========================================================================
// HwTextureManagerFactory
//

HwTextureManagerUPtr HwTextureManagerFactory::create(
	RendererPtr renderer,
	SpriteCachePtr sprite_cache)
{
	auto result = RendererTextureManagerImplUPtr{new RendererTextureManagerImpl{}};

	result->initialize(renderer, sprite_cache);

	return result;
}

//
// HwTextureManagerFactory
// ==========================================================================


} // bstone
