/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// Missing sprite image (64x64).
//


#ifndef BSTONE_MISSING_SPRITE_64X64_IMAGE_INCLUDED
#define BSTONE_MISSING_SPRITE_64X64_IMAGE_INCLUDED


#include <array>


namespace bstone
{


using MissingSpriteImage = std::array<unsigned char, 4 * 64 * 64>;


const MissingSpriteImage& get_missing_sprite_image();


} // bstone


#endif // !BSTONE_MISSING_SPRITE_64X64_IMAGE_INCLUDED
