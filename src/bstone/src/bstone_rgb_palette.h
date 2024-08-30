/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Palette types of RGB color model.
//


#ifndef BSTONE_RGB_PALETTE_INCLUDED
#define BSTONE_RGB_PALETTE_INCLUDED


#include <array>

#include "bstone_rgb8.h"


namespace bstone
{


struct RgbPalette
{
	static constexpr int get_max_color_count() noexcept
	{
		return 256;
	}
}; // RgbPalette


using Rgba8Palette = std::array<Rgba8, RgbPalette::get_max_color_count()>;
using Rgba8PalettePtr = Rgba8Palette*;
using Rgba8PaletteCPtr = const Rgba8Palette*;


} // bstone


#endif // !BSTONE_RGB_PALETTE_INCLUDED
