//
// Sprite cache.
//


#include "bstone_sprite_cache.h"
#include <stdexcept>
#include "id_pm.h"


namespace bstone
{


SpriteCache::SpriteCache()
	:
	cache_{max_sprites}
{
}

SpriteCache::SpriteCache(
	SpriteCache&& rhs)
	:
	cache_{std::move(rhs.cache_)}
{
}

const Sprite* SpriteCache::cache(
	const int sprite_id)
{
	if (sprite_id <= 0 || sprite_id >= max_sprites)
	{
		throw std::runtime_error{"Invalid sprite id."};
	}

	const auto sprite_data = ::PM_GetSpritePage(sprite_id);

	if (!sprite_data)
	{
		throw std::runtime_error{"No sprite data."};
	}

	auto& sprite = cache_[sprite_id];

	if (!sprite.is_initialized())
	{
		sprite.initialize(sprite_data);
	}

	return &sprite;
}


} // bstone
