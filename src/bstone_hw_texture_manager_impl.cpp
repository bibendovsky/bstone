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
#include <unordered_map>
#include "id_pm.h"
#include "bstone_hw_texture_manager.h"
#include "bstone_missing_sprite_64x64_image.h"
#include "bstone_missing_wall_64x64_image.h"
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

	struct Texture2dItem
	{
		GenerationId generation_id_;
		RendererTexture2dPtr texture_2d_;
	}; // Texture2dItem

	using IdToTexture2dMap = std::unordered_map<int, Texture2dItem>;


	bool is_initialized_;
	mutable std::string error_message_;

	RendererPtr renderer_;
	SpriteCachePtr sprite_cache_;

	bool is_caching_;
	GenerationId generation_id_;

	IdToTexture2dMap wall_map_;
	IdToTexture2dMap sprite_map_;

	RendererTexture2dPtr missing_sprite_texture_2d_;
	RendererTexture2dPtr missing_wall_texture_2d_;


	void destroy_missing_sprite_texture();

	bool create_missing_sprite_texture();


	void destroy_missing_wall_texture();

	bool create_missing_wall_texture();


	RendererTexture2dPtr wall_create_texture(
		const int wall_id);

	RendererTexture2dPtr sprite_create_texture(
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
	missing_sprite_texture_2d_{},
	missing_wall_texture_2d_{}
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
	missing_sprite_texture_2d_{std::move(rhs.missing_sprite_texture_2d_)},
	missing_wall_texture_2d_{std::move(rhs.missing_wall_texture_2d_)}
{
	rhs.is_initialized_ = false;
	rhs.missing_sprite_texture_2d_ = nullptr;
	rhs.missing_wall_texture_2d_ = nullptr;
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

	auto texture_2d = wall_create_texture(id);

	if (texture_2d == nullptr)
	{
		return false;
	}

	auto texture_2d_item = Texture2dItem{};
	texture_2d_item.generation_id_ = generation_id_;
	texture_2d_item.texture_2d_ = texture_2d;

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

	auto texture_2d = sprite_create_texture(id);

	if (texture_2d == nullptr)
	{
		return false;
	}

	auto texture_2d_item = Texture2dItem{};
	texture_2d_item.generation_id_ = generation_id_;
	texture_2d_item.texture_2d_ = texture_2d;

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
		auto& texture_2d = sprite_item.second.texture_2d_;

		renderer_->texture_2d_destroy(texture_2d);
		texture_2d = nullptr;

		texture_2d = sprite_create_texture(sprite_id);

		if (texture_2d == nullptr)
		{
			return false;
		}
	}

	// Walls.
	//
	for (auto& wall_item : wall_map_)
	{
		const auto wall_id = wall_item.first;
		auto& texture_2d = wall_item.second.texture_2d_;

		renderer_->texture_2d_destroy(texture_2d);
		texture_2d = nullptr;

		texture_2d = wall_create_texture(wall_id);

		if (texture_2d == nullptr)
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

	destroy_missing_sprite_texture();
	destroy_missing_wall_texture();

	renderer_ = nullptr;
	sprite_cache_ = nullptr;
}

void RendererTextureManagerImpl::Detail::destroy_missing_sprite_texture()
{
	if (missing_sprite_texture_2d_ == nullptr)
	{
		return;
	}

	renderer_->texture_2d_destroy(missing_sprite_texture_2d_);
	missing_sprite_texture_2d_ = nullptr;
}

bool RendererTextureManagerImpl::Detail::create_missing_sprite_texture()
{
	destroy_missing_sprite_texture();

	const auto& raw_image = get_missing_sprite_image();
	const auto rgba_image = reinterpret_cast<const RendererColor32*>(raw_image.data());

	auto param = RendererTexture2dCreateParam{};
	param.internal_format_ = RendererPixelFormat::r8g8b8a8;
	param.has_mipmaps_ = true;
	param.width_ = Sprite::dimension;
	param.height_ = Sprite::dimension;
	param.rgba_pixels_ = rgba_image;

	auto texture_2d = renderer_->texture_2d_create(param);

	if (texture_2d == nullptr)
	{
		error_message_ = "Failed to create a missing sprite texture. ";
		error_message_ += renderer_->get_error_message();

		return false;
	}

	// TODO
	texture_2d->generate_mipmaps();

	missing_sprite_texture_2d_ = texture_2d;

	return true;
}

void RendererTextureManagerImpl::Detail::destroy_missing_wall_texture()
{
	if (missing_wall_texture_2d_ == nullptr)
	{
		return;
	}

	renderer_->texture_2d_destroy(missing_wall_texture_2d_);
	missing_wall_texture_2d_ = nullptr;
}

bool RendererTextureManagerImpl::Detail::create_missing_wall_texture()
{
	destroy_missing_wall_texture();

	const auto& raw_image = get_missing_wall_image();
	const auto rgba_image = reinterpret_cast<const RendererColor32*>(raw_image.data());

	auto param = RendererTexture2dCreateParam{};
	param.internal_format_ = RendererPixelFormat::r8g8b8;
	param.has_mipmaps_ = true;
	param.width_ = wall_dimension;
	param.height_ = wall_dimension;
	param.rgba_pixels_ = rgba_image;

	auto texture_2d = renderer_->texture_2d_create(param);

	if (texture_2d == nullptr)
	{
		error_message_ = "Failed to create a missing wall texture. ";
		error_message_ += renderer_->get_error_message();

		return false;
	}

	// TODO
	texture_2d->generate_mipmaps();

	missing_wall_texture_2d_ = texture_2d;

	return true;
}

RendererTexture2dPtr RendererTextureManagerImpl::Detail::wall_create_texture(
	const int wall_id)
{
	const auto indexed_pixels = static_cast<const std::uint8_t*>(::PM_GetPage(wall_id));

	if (indexed_pixels == nullptr)
	{
		error_message_ = "Null data.";

		return nullptr;
	}

	auto param = RendererTexture2dCreateParam{};
	param.internal_format_ = RendererPixelFormat::r8g8b8;
	param.has_mipmaps_ = true;
	param.indexed_is_column_major_ = true;
	param.width_ = wall_dimension;
	param.height_ = wall_dimension;
	param.indexed_pixels_ = indexed_pixels;

	auto texture_2d = renderer_->texture_2d_create(param);

	if (texture_2d == nullptr)
	{
		error_message_ = "Failed to create a wall texture. ";
		error_message_ += renderer_->get_error_message();

		return nullptr;
	}

	// TODO
	texture_2d->generate_mipmaps();

	return texture_2d;
}

RendererTexture2dPtr RendererTextureManagerImpl::Detail::sprite_create_texture(
	const int sprite_id)
{
	auto sprite = sprite_cache_->cache(sprite_id);

	if (sprite == nullptr)
	{
		error_message_ = "Failed to cache a sprite #" + std::to_string(sprite_id) + ".";

		return nullptr;
	}

	if (!sprite->is_initialized())
	{
		error_message_ = "Sprite #" + std::to_string(sprite_id) + " not initialized.";

		return nullptr;
	}

	auto param = RendererTexture2dCreateParam{};
	param.internal_format_ = RendererPixelFormat::r8g8b8a8;
	param.has_mipmaps_ = true;
	param.width_ = Sprite::dimension;
	param.height_ = Sprite::dimension;
	param.indexed_sprite_ = sprite;

	auto texture_2d = renderer_->texture_2d_create(param);

	if (texture_2d == nullptr)
	{
		error_message_ = "Failed to create a sprite texture. ";
		error_message_ += renderer_->get_error_message();

		return nullptr;
	}

	// TODO
	texture_2d->generate_mipmaps();

	return texture_2d;
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
			return missing_sprite_texture_2d_;

		case ImageKind::wall:
			return missing_wall_texture_2d_;

		default:
			return nullptr;
		}
	}

	return item_it->second.texture_2d_;
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
