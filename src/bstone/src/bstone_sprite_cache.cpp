/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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


SpriteCache::SpriteCache()
	:
	cache_{max_sprites}
{
}

SpriteCache::SpriteCache(
	SpriteCache&& rhs) noexcept
	:
	cache_{std::move(rhs.cache_)}
{
}

const Sprite* SpriteCache::cache(
	const int sprite_id)
{
	if (sprite_id <= 0 || sprite_id >= max_sprites)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid sprite id.");
	}

	const auto sprite_data = globals::page_mgr->get_sprite(sprite_id);

	if (!sprite_data)
	{
		BSTONE_THROW_STATIC_SOURCE("No sprite data.");
	}

	auto& sprite = cache_[sprite_id];

	if (!sprite.is_initialized())
	{
		sprite.initialize(sprite_data);
	}

	return &sprite;
}


} // bstone
