/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Former "D3_DASM2.ASM".

#include <cstdint>

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

namespace {

enum class DrawMode
{
	ceiling,
	floor,
	ceiling_and_floor,
};

enum class ShadingMode
{
	none,
	standard,
};

void map_row(DrawMode draw_mode, ShadingMode shading_mode)
{
	const auto need_to_draw_ceiling =
		draw_mode == DrawMode::ceiling ||
		draw_mode == DrawMode::ceiling_and_floor;

	const auto need_to_draw_floor =
		draw_mode == DrawMode::floor ||
		draw_mode == DrawMode::ceiling_and_floor;

	const auto has_shading = shading_mode == ShadingMode::standard;

	const auto xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
	auto xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

	for (auto i = 0; i < mr_count; ++i)
	{
		const auto xy = ((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E);

		if (need_to_draw_ceiling)
		{
			const auto ceiling_index = planepics[xy % 8192];
			const auto ceiling_pixel = has_shading ? shadingtable[ceiling_index] : ceiling_index;
			vga_memory[mr_dest + i] = ceiling_pixel;
		}

		if (need_to_draw_floor)
		{
			const auto floor_index = planepics[(xy + 1) % 8192];
			const auto floor_pixel = has_shading ? shadingtable[floor_index] : floor_index;
			vga_memory[mr_dest + mr_rowofs + i] = floor_pixel;
		}

		xy_frac += xy_step;
	}
}

} // namespace

void MapLSRow()
{
	map_row(DrawMode::ceiling_and_floor, ShadingMode::standard);
}

void F_MapLSRow()
{
	map_row(DrawMode::floor, ShadingMode::standard);
}

void C_MapLSRow()
{
	map_row(DrawMode::ceiling, ShadingMode::standard);
}

void MapRow()
{
	map_row(DrawMode::ceiling_and_floor, ShadingMode::none);
}

void F_MapRow()
{
	map_row(DrawMode::floor, ShadingMode::none);
}

void C_MapRow()
{
	map_row(DrawMode::ceiling, ShadingMode::none);
}
