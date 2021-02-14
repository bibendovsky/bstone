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


#include "3d_def.h"
#include "id_pm.h"
#include "id_vl.h"

#include "bstone_math.h"


#define MAXVIEWHEIGHT (200)


void MapLSRow();


std::uint16_t CeilingTile = 126, FloorTile = 126;

void(*MapRowPtr)();

SpanStart spanstart;
BaseDist basedist;
PlaneYLookup planeylookup;
MirrorOfs mirrorofs;

extern std::uint8_t planepics[8192]; // 4k of ceiling, 4k of floor
extern const std::uint8_t* lightsource;
extern const std::uint8_t* shadingtable;

int halfheight = 0;

double psin;
double pcos;

fixed FixedMul(
	fixed a,
	fixed b)
{
	return (a >> 8) * (b >> 8);
}


int mr_rowofs = 0;
int mr_count = 0;
int mr_xstep = 0;
int mr_ystep = 0;
int mr_xfrac = 0;
int mr_yfrac = 0;
int mr_dest = 0;


/*
==============
=
= DrawSpans
=
= Height ranges from 0 (infinity) to viewheight/2 (nearest)
==============
*/
void DrawSpans(
	int x1,
	int x2,
	int height)
{
	const auto toprow = bufferofs + planeylookup[height];

	mr_rowofs = mirrorofs[height];

	mr_xstep = bstone::math::floating_to_fixed(psin / (2 * height));
	mr_ystep = bstone::math::floating_to_fixed(pcos / (2 * height));

	const auto length = basedist[height];
	const auto startxfrac = bstone::math::floating_to_fixed(viewx + (length * pcos));
	const auto startyfrac = bstone::math::floating_to_fixed(viewy - (length * psin));

	if (!gp_no_shading_)
	{
		auto i = shade_max - ((63 * height) / normalshade);

		if (i < 0)
		{
			i = 0;
		}
		else if (i > 63)
		{
			i = 63;
		}

		shadingtable = lightsource + (i * 256);
	}

	const auto prestep = (viewwidth / 2) - x1;

	mr_xfrac = startxfrac - (mr_xstep * prestep);
	mr_yfrac = startyfrac - (mr_ystep * prestep);

	mr_dest = toprow + x1;
	mr_count = x2 - x1 + 1;

	if (mr_count > 0)
	{
		MapRowPtr();
	}
}

void SetPlaneViewSize()
{
	const std::uint8_t* src;
	std::uint8_t* dest;

	halfheight = viewheight / 2;

	for (int y = 0; y < halfheight; ++y)
	{
		planeylookup[y] = (halfheight - 1 - y) * vga_width;
		mirrorofs[y] = (y * 2 + 1) * vga_width;

		if (y > 0)
		{
			basedist[y] = 1.0 / 2.0 * scale_ / y;
		}
	}

	src = static_cast<const std::uint8_t*>(PM_GetPage(CeilingTile));
	dest = planepics;

	for (int x = 0; x < 4096; ++x)
	{
		*dest = *src++;
		dest += 2;
	}

	src = static_cast<const std::uint8_t*>(PM_GetPage(FloorTile));
	dest = planepics + 1;

	for (int x = 0; x < 4096; ++x)
	{
		*dest = *src++;
		dest += 2;
	}
}

void DrawPlanes()
{
	if (vid_is_hw_)
	{
		return;
	}

	if ((viewheight / 2) != halfheight)
	{
		SetPlaneViewSize(); // screen size has changed
	}

	psin = viewsin;
	pcos = viewcos;

	auto lastheight_d = static_cast<double>(halfheight);

	int x = 0;

	for ( ; x < viewwidth; ++x)
	{
		const auto height_d = wallheight[x] / 8.0;

		if (height_d < lastheight_d)
		{
			auto height = static_cast<int>(height_d);
			auto lastheight = static_cast<int>(lastheight_d);

			// more starts
			do
			{
				spanstart[--lastheight] = x;
			} while (lastheight > height);

			lastheight_d = static_cast<double>(lastheight);
		}
		else if (height_d > lastheight_d)
		{
			const auto height = static_cast<int>(std::min(height_d, static_cast<double>(halfheight)));
			auto lastheight = static_cast<int>(lastheight_d);

			// draw spans
			while (lastheight < height)
			{
				if (lastheight > 0)
				{
					DrawSpans(spanstart[lastheight], x - 1, lastheight);
				}

				++lastheight;
			}

			lastheight_d = static_cast<double>(lastheight);
		}
	}

	for (auto lastheight = static_cast<int>(lastheight_d); lastheight < halfheight; ++lastheight)
	{
		if (lastheight > 0)
		{
			DrawSpans(spanstart[lastheight], x - 1, lastheight);
		}
	}
}
