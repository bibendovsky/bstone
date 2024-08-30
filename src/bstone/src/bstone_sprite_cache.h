/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Sprite cache.
//


#ifndef BSTONE_SPRITE_CACHE_INCLUDED
#define BSTONE_SPRITE_CACHE_INCLUDED


#include <vector>

#include "bstone_sprite.h"


namespace bstone
{


class SpriteCache
{
public:
	static constexpr auto max_sprites = 1'000;


	SpriteCache();

	SpriteCache(
		SpriteCache&& rhs) noexcept;

	SpriteCache(
		const SpriteCache& rhs) = delete;

	SpriteCache& operator=(
		const SpriteCache& rhs) = delete;


	const Sprite* cache(
		const int sprite_id);


private:
	using Cache = std::vector<Sprite>;


	Cache cache_;
}; // SpriteCache

using SpriteCachePtr = SpriteCache*;


} // bstone


#endif // !BSTONE_SPRITE_CACHE_INCLUDED
