/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "3d_def.h"
#include "id_vl.h"
#include "bstone_sprite.h"
#include "bstone_sprite_cache.h"


extern bstone::SpriteCache vid_sprite_cache;


enum class ShapeDrawMode
{
	simple,
	shaded,
	player_weapon,
}; // ShapeDrawMode


extern const std::uint8_t* shadingtable;
extern const std::uint8_t* lightsource;


#define CLOAKED_SHAPES (1)


/*
=============================================================================

 GLOBALS

=============================================================================
*/

int maxscale;
int maxscaleshl2;
int centery;

int normalshade;
int normalshade_div = 1;
int shade_max = 1;

std::int16_t nsd_table[] = {1, 6, 3, 4, 1, 2};
std::int16_t sm_table[] = {36, 51, 62, 63, 18, 52};
std::uint16_t* linecmds;


void SetupScaling(
	int maxscaleheight) noexcept
{
	maxscaleheight /= 2; // one scaler every two pixels

	maxscale = maxscaleheight - 1;
	maxscaleshl2 = maxscale * 4;
	update_normalshade();
	centery = viewheight / 2;
}


extern bool useBounceOffset;

double bounceOffset = 0.0;

void generic_scale_shape(
	const int xcenter,
	const int shapenum,
	const int ref_height,
	const std::int8_t lighting,
	const ShapeDrawMode draw_mode)
{
	const auto is_player_weapon = (draw_mode == ShapeDrawMode::player_weapon);

	if (!is_player_weapon)
	{
		const auto ref_half_height = ref_height / 2;

		if (ref_half_height == 0)
		{
			return;
		}

		if (ref_half_height > maxscaleshl2)
		{
			return;
		}
	}

	const auto height =
		is_player_weapon ?
		(vga_height * ref_height) / vga_ref_height :
		ref_height / 4;

	if (height == 0)
	{
		return;
	}

	constexpr auto dimension = bstone::Sprite::dimension;

	constexpr auto mid_bob = 6;
	constexpr auto bob_start = 6;

	const auto use_bobbing = is_player_weapon && useBounceOffset;

	const auto bounce_offset_n =
		use_bobbing ?
		static_cast<int>(bounceOffset) :
		0;

	const auto bob_offset =
		use_bobbing ?
		(vga_height * (bob_start + mid_bob - bounce_offset_n)) / vga_ref_height :
		0;


	const auto sprite_ptr = vid_sprite_cache.cache(shapenum);
	const auto sprite_width = sprite_ptr->get_width();
	const auto sprite_height = sprite_ptr->get_height();

	const auto half_height = height / 2;

	const auto offset_x =
		is_player_weapon ?
		(viewwidth - height) / 2 :
		xcenter - half_height;

	const auto offset_y =
		is_player_weapon ?
		vga_3d_view_bottom_y - height + bob_offset :
		vga_3d_view_top_y + centery - half_height;

	const auto left = sprite_ptr->get_left();
	auto x1 = offset_x + ((left * height) / dimension);

	if (x1 >= viewwidth)
	{
		return;
	}

	auto x2 = x1 + ((sprite_width * height) / dimension);

	const auto top = sprite_ptr->get_top();
	auto y1 = offset_y + ((top * height) / dimension);

	if (y1 > vga_3d_view_bottom_y)
	{
		return;
	}

	auto y2 = y1 + ((sprite_height * height) / dimension);


	const auto tx_delta = static_cast<double>(dimension) / static_cast<double>(height);

	auto tx_column = 0.0;

	if (x1 < 0)
	{
		tx_column -= tx_delta * x1;
		x1 = 0;
	}

	if (x2 > viewwidth)
	{
		x2 = viewwidth;
	}

	if (x2 < x1)
	{
		return;
	}

	auto tx_row_begin = 0.0;

	if (y1 < vga_3d_view_top_y)
	{
		tx_row_begin += tx_delta * (vga_3d_view_top_y - y1);
		y1 = vga_3d_view_top_y;
	}

	if (y2 > vga_3d_view_bottom_y)
	{
		y2 = vga_3d_view_bottom_y;
	}

	if (y2 < y1)
	{
		return;
	}

	const std::uint8_t* shading = nullptr;

	if (draw_mode == ShapeDrawMode::shaded)
	{
		auto i = shade_max - (63 * ref_height / (normalshade * 8)) + lighting;

		if (i < 0)
		{
			i = 0;
		}
		else if (i > 63)
		{
			i = 63;
		}

		// BBi Don't shade cloaked shape
		if (cloaked_shape)
		{
			i = 0;
		}

		shading = lightsource + (i * 256);
	}


	for (int x = x1; x < x2; ++x)
	{
		if (!is_player_weapon && wallheight[x] > static_cast<double>(ref_height))
		{
			tx_column += tx_delta;
			continue;
		}

		const auto column_index = static_cast<int>(tx_column);
		const auto column = sprite_ptr->get_column(column_index);
		auto tx_row = tx_row_begin;

		for (int y = y1; y < y2; ++y)
		{
			const auto row_index = static_cast<int>(tx_row);
			const auto sprite_color = column[row_index];

			if (sprite_color < 0)
			{
				tx_row += tx_delta;
				continue;
			}

			const auto pixel_offset = vl_get_offset(0, x, y);
			auto color_index = static_cast<std::uint8_t>(sprite_color);

			if (draw_mode == ShapeDrawMode::shaded)
			{
#if CLOAKED_SHAPES
				if (cloaked_shape)
				{
					color_index = shading[0x1000 | vga_memory[pixel_offset]];
				}
				else
#endif
				{
					color_index = shading[color_index];
				}
			}

			vga_memory[pixel_offset] = color_index;

			tx_row += tx_delta;
		}

		tx_column += tx_delta;
	}
}

/*
=======================
=
= ScaleLSShape with Light sourcing
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
=       end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
=       top of virtual line with segment in proper place
=       start of segment pixel*2, used to jsl into compiled scaler
=       <repeat>
=
= Setup for call
= --------------
= GC_MODE read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX pointing at GC_BITMASK
=
=======================
*/
void ScaleLSShape(
	int xcenter,
	int shapenum,
	int height,
	std::int8_t lighting)
{
	generic_scale_shape(
		xcenter,
		shapenum,
		height,
		lighting,
		ShapeDrawMode::shaded);
}

/*
=======================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
= each vertical line of the shape has a pointer to segment data:
=       end of segment pixel*2 (0 terminates line) used to patch rtl in scaler
=       top of virtual line with segment in proper place
=       start of segment pixel*2, used to jsl into compiled scaler
=       <repeat>
=
= Setup for call
= --------------
= GC_MODE read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX pointing at GC_BITMASK
=
=======================
*/
void ScaleShape(
	int xcenter,
	int shapenum,
	int height)
{
	generic_scale_shape(
		xcenter,
		shapenum,
		height,
		0,
		ShapeDrawMode::simple);
}

// BBi
void scale_player_weapon(
	const int sprite_id,
	const int height)
{
	generic_scale_shape(
		0,
		sprite_id,
		height,
		0,
		ShapeDrawMode::player_weapon);
}

void update_normalshade() noexcept
{
	normalshade = static_cast<int>(
		(3.0 * maxscale) / (4.0 * normalshade_div) / vga_wide_scale);
}
// BBi
