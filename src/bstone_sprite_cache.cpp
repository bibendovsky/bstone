/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Sprite cache.
//


#include "bstone_sprite_cache.h"

#include "bstone_exception.h"
#include "bstone_globals.h"


namespace bstone
{


namespace
{


class SpriteCacheException :
	public Exception
{
public:
	explicit SpriteCacheException(
		const char* message) noexcept
		:
		Exception{"SPRITE_CACHE", message}
	{
	}
}; // SpriteCacheException


} // namespace


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
		fail("Invalid sprite id.");
	}

	const auto sprite_data = globals::page_mgr->get_sprite(sprite_id);

	if (!sprite_data)
	{
		fail("No sprite data.");
	}

	auto& sprite = cache_[sprite_id];

	if (!sprite.is_initialized())
	{
		sprite.initialize(sprite_data);
	}

	return &sprite;
}

[[noreturn]]
void SpriteCache::fail(
	const char* message)
{
	throw SpriteCacheException{message};
}

[[noreturn]]
void SpriteCache::fail_nested(
	const char* message)
{
	std::throw_with_nested(SpriteCacheException{message});
}


} // bstone
