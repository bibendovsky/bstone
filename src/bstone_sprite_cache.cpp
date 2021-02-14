/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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

	const auto sprite_data = PM_GetSpritePage(sprite_id);

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
