/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Former D3_DASM2.ASM.
//


#include <cmath>
#include <cstdint>


enum DrawOptions
{
	DO_CEILING,
	DO_FLOORING,
	DO_CEILING_AND_FLOORING
}; // DrawOptions

enum ShadingOptions
{
	SO_NONE,
	SO_DEFAULT
}; // ShadingOptions


extern int mr_rowofs;
extern int mr_count;
extern int mr_xstep;
extern int mr_ystep;
extern int mr_xfrac;
extern int mr_yfrac;
extern int mr_dest;

extern const std::uint8_t* shadingtable;
extern std::uint8_t* vga_memory;


std::uint8_t planepics[8192]; // 4k of ceiling, 4k of floor


static void generic_map_row(
	DrawOptions draw_options,
	ShadingOptions shading_options)
{
	const auto need_draw_ceiling =
		draw_options == DO_CEILING ||
		draw_options == DO_CEILING_AND_FLOORING;

	const auto need_draw_flooring =
		draw_options == DO_FLOORING ||
		draw_options == DO_CEILING_AND_FLOORING;

	auto xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
	auto xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

	auto screen_offset = mr_dest;

	for (int i = 0; i < mr_count; ++i)
	{
		auto xy = ((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E);

		if (need_draw_ceiling)
		{
			const auto ceiling_index = planepics[xy % 8192];

			const auto ceiling_pixel =
				(shading_options == SO_DEFAULT) ?
				shadingtable[ceiling_index] :
				ceiling_index;

			vga_memory[screen_offset] = ceiling_pixel;
		}

		if (need_draw_flooring)
		{
			const auto flooring_index = planepics[(xy + 1) % 8192];

			const auto flooring_pixel =
				(shading_options == SO_DEFAULT) ?
				shadingtable[flooring_index] :
				flooring_index;

			vga_memory[screen_offset + mr_rowofs] = flooring_pixel;
		}

		screen_offset += 1;
		xy_frac += xy_step;
	}
}

void MapLSRow()
{
	generic_map_row(DO_CEILING_AND_FLOORING, SO_DEFAULT);
}

void F_MapLSRow()
{
	generic_map_row(DO_FLOORING, SO_DEFAULT);
}

void C_MapLSRow()
{
	generic_map_row(DO_CEILING, SO_DEFAULT);
}

void MapRow()
{
	generic_map_row(DO_CEILING_AND_FLOORING, SO_NONE);
}

void F_MapRow()
{
	generic_map_row(DO_FLOORING, SO_NONE);
}

void C_MapRow()
{
	generic_map_row(DO_CEILING, SO_NONE);
}
