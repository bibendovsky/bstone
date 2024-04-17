/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Former "D3_DASM2.ASM".

#include "d3_d2.h"

extern int mr_rowofs;
extern int mr_count;
extern int mr_xstep;
extern int mr_ystep;
extern int mr_xfrac;
extern int mr_yfrac;
extern int mr_dest;

extern const std::uint8_t* shadingtable;
extern std::uint8_t* vga_memory;

std::uint8_t planepics[planepics_size]; // 4k of ceiling, 4k of floor

namespace {

struct NoShadingFunc
{
	std::uint8_t operator()(std::uint8_t pixel) const noexcept
	{
		return pixel;
	}
};

struct StandardShadingFunc
{
	std::uint8_t operator()(std::uint8_t pixel) const noexcept
	{
		return shadingtable[pixel];
	}
};

template<typename TShadingFunc>
struct NoDrawFunc
{
	void operator()(int, int) const noexcept {}
};

template<typename TShadingFunc>
struct CeilingDrawFunc
{
	void operator()(int xy, int i_mr) const noexcept
	{
		const auto ceiling_index = planepics[xy % planepics_size];
		const auto ceiling_pixel = TShadingFunc{}(ceiling_index);
		vga_memory[mr_dest + i_mr] = ceiling_pixel;
	}
};

template<typename TShadingFunc>
struct FloorDrawFunc
{
	void operator()(int xy, int i_mr) const noexcept
	{
		const auto floor_index = planepics[(xy + 1) % planepics_size];
		const auto floor_pixel = TShadingFunc{}(floor_index);
		vga_memory[mr_dest + mr_rowofs + i_mr] = floor_pixel;
	}
};

template<typename TCeilingDrawFunc, typename TFloorDrawFunc>
void map_row()
{
	const auto xy_step = (mr_ystep << 16) | (mr_xstep & 0xFFFF);
	auto xy_frac = (mr_yfrac << 16) | (mr_xfrac & 0xFFFF);

	for (auto i = 0; i < mr_count; ++i)
	{
		const auto xy = ((xy_frac >> 3) & 0x1FFF1F80) | ((xy_frac >> 25) & 0x7E);
		TCeilingDrawFunc{}(xy, i);
		TFloorDrawFunc{}(xy, i);
		xy_frac += xy_step;
	}
}

} // namespace

void MapLSRow()
{
	using ShadingFunc = StandardShadingFunc;
	using CeilingFunc = CeilingDrawFunc<ShadingFunc>;
	using FloorFunc = FloorDrawFunc<ShadingFunc>;

	map_row<CeilingFunc, FloorFunc>();
}

void F_MapLSRow()
{
	using ShadingFunc = StandardShadingFunc;
	using CeilingFunc = NoDrawFunc<ShadingFunc>;
	using FloorFunc = FloorDrawFunc<ShadingFunc>;

	map_row<CeilingFunc, FloorFunc>();
}

void C_MapLSRow()
{
	using ShadingFunc = StandardShadingFunc;
	using CeilingFunc = CeilingDrawFunc<ShadingFunc>;
	using FloorFunc = NoDrawFunc<ShadingFunc>;

	map_row<CeilingFunc, FloorFunc>();
}

void MapRow()
{
	using ShadingFunc = NoShadingFunc;
	using CeilingFunc = CeilingDrawFunc<ShadingFunc>;
	using FloorFunc = FloorDrawFunc<ShadingFunc>;

	map_row<CeilingFunc, FloorFunc>();
}

void F_MapRow()
{
	using ShadingFunc = NoShadingFunc;
	using CeilingFunc = NoDrawFunc<ShadingFunc>;
	using FloorFunc = FloorDrawFunc<ShadingFunc>;

	map_row<CeilingFunc, FloorFunc>();
}

void C_MapRow()
{
	using ShadingFunc = NoShadingFunc;
	using CeilingFunc = CeilingDrawFunc<ShadingFunc>;
	using FloorFunc = NoDrawFunc<ShadingFunc>;

	map_row<CeilingFunc, FloorFunc>();
}
