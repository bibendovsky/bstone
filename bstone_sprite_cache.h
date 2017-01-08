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
    static constexpr int max_sprites = 1000;


    SpriteCache();

    SpriteCache(
        const SpriteCache& that) = delete;

    SpriteCache& operator=(
        const SpriteCache& that) = delete;

    ~SpriteCache();


    const Sprite* cache(
        const int sprite_id,
        const void* sprite_data);


private:
    using Cache = std::vector<Sprite>;


    Cache cache_;
}; // SpriteCache


} // bstone


#endif // !BSTONE_SPRITE_CACHE_INCLUDED
