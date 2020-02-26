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
		SpriteCache&& rhs);

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
