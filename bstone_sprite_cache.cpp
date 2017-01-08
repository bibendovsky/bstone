//
// Sprite cache.
//


#include "bstone_sprite_cache.h"


namespace bstone
{


SpriteCache::SpriteCache() :
        cache_{ max_sprites }
{
}

SpriteCache::~SpriteCache()
{
}

const Sprite* SpriteCache::cache(
    const int sprite_id,
    const void* sprite_data)
{
    if (sprite_id <= 0 || sprite_id >= max_sprites)
    {
        throw "Invalid sprite id.";
    }

    if (!sprite_data)
    {
        throw "No sprite data.";
    }

    auto& sprite = cache_[sprite_id];

    if (!sprite.is_initialized())
    {
        sprite.initialize(sprite_data);
    }

    return &sprite;
}


} // bstone
