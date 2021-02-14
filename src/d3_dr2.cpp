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


//
// Former D3_DR_A2.ASM
//
// To emulate D3_DR_A.ASM define W3D.
//


#include <cassert>
#include <cmath>

#include <array>


#include "3d_def.h"
#include "id_vl.h"


static const int DEG90 = 900;
static const int DEG180 = 1800;
static const int DEG270 = 2700;
static const int DEG360 = 3600;


void HitHorizWall();
void HitVertWall();
void HitHorizDoor();
void HitVertDoor();
void HitHorizPWall();
void HitVertPWall();



extern FineTangent finetangent;
extern int viewwidth;
extern TileMap tilemap;
extern SpotVis spotvis;
extern std::vector<int> pixelangle;
extern int midangle;
extern int focaltx;
extern int focalty;
extern int viewtx;
extern int viewty;
extern double viewx;
extern double viewy;
extern double xpartialup;
extern double xpartialdown;
extern double ypartialup;
extern double ypartialdown;
extern int tilehit;
extern int pixx;
extern WallHeight wallheight;
extern int xtile;
extern int ytile;
extern int xtilestep;
extern int ytilestep;
extern double xintercept;
extern double yintercept;
double xstep;
double ystep;
extern DoorPositions doorposition;
extern double pwallpos;


double get_integral_1(
	const double value) noexcept
{
	return value < 0.0 ? std::floor(value) : std::trunc(value);
}

double get_fractional_1(
	const double value) noexcept
{
	double integral;
	return std::modf(1.0 + value, &integral);
}

void AsmRefresh()
{
	double xpartial;
	double ypartial;
	double intercept;
	int intercept_h; // high part of intercept
	double intercept_l; // low part of intercept
	int xs_x; // temporary xspot (x)
	int xs_y; // temporary xspot (y)
	int xt; // temporary xtile
	double xint; // temporary xintercept
	int xint_h; // high part of temporary xintercept
	int ys_x; // temporary yspot (x)
	int ys_y; // temporary yspot (y)
	int yt; // temporary ytile
	double yint; // temporary yintercept
	int yint_h; // high part of temporary yintercept

	vid_hw_clear_wall_render_list();
	vid_hw_clear_pushwall_render_list();
	vid_hw_clear_door_render_list();

	pixx = 0;

pixxloop:

	int angle = midangle + pixelangle[pixx];

	while (angle < 0)
	{
		angle += FINEANGLES;
	}

	while (angle >= DEG360)
	{
		angle -= FINEANGLES;
	}

	if (angle >= DEG270)
	{
		xtilestep = 1;
		ytilestep = 1;

		xstep = finetangent[angle - DEG270];
		ystep = finetangent[DEG360 - 1 - angle];

		xpartial = xpartialup;
		ypartial = ypartialup;
	}
	else if (angle >= DEG180)
	{
		xtilestep = -1;
		ytilestep = 1;

		xstep = -finetangent[DEG270 - 1 - angle];
		ystep = finetangent[angle - DEG180];

		xpartial = xpartialdown;
		ypartial = ypartialup;
	}
	else if (angle >= DEG90)
	{
		xtilestep = -1;
		ytilestep = -1;

		xstep = -finetangent[angle - DEG90];
		ystep = -finetangent[DEG180 - 1 - angle];

		xpartial = xpartialdown;
		ypartial = ypartialdown;
	}
	else
	{
		xtilestep = 1;
		ytilestep = -1;

		xstep = finetangent[DEG90 - 1 - angle];
		ystep = -finetangent[angle];

		xpartial = xpartialup;
		ypartial = ypartialdown;
	}


	//
	// initialise variables for intersection testing
	//

	yintercept = viewy + (ystep * xpartial);
	yint_h = static_cast<int>(yintercept);

	xtile = focaltx + xtilestep;
	xs_x = xtile;
	xs_y = yint_h;

	xint = viewx + (xstep * ypartial);
	xint_h = static_cast<int>(get_integral_1(xint));

	xintercept = get_fractional_1(xint);

	yt = focalty + ytilestep;
	ys_x = xint_h;
	ys_y = yt;

	xt = xtile;

	//
	// trace along this angle until we hit a wall
	//
	// CORE LOOP!
	//

	//
	// check intersections with vertical walls
	//

vertcheck:

	if ((ytilestep < 0 && yint_h <= yt) || (ytilestep > 0 && yint_h >= yt))
	{
		goto horizentry;
	}

vertentry:

	if (tilemap[xs_x][xs_y] != 0)
	{
		tilehit = tilemap[xs_x][xs_y];

		if ((tilehit & 0x80) != 0)
		{
			//
			// hit a special vertical wall, so find which coordinate a
			// door would be intersected at, and check to see if the
			// door is open past that point
			//

			xtile = xt;

			yintercept = yint_h + get_fractional(yintercept);

			if ((tilehit & 0x40) != 0)
			{
				//
				// hit a sliding vertical wall
				//

				intercept = ystep * pwallpos;
				yint = get_fractional(intercept) + get_fractional(yintercept);
				intercept_l = get_fractional_1(yint);
				intercept_h = yint_h + static_cast<int>(intercept + get_integral_1(yint));

				if (yint_h == intercept_h)
				{
					//
					// draw the pushable wall at the new height
					//

					yintercept = intercept_h + intercept_l;
					xintercept = xt;

					HitVertPWall();

					goto nextpix;
				}
			}
			else
			{
				intercept = yintercept + (0.5 * ystep);
				intercept_l = get_fractional(intercept);
				intercept_h = static_cast<int>(intercept);

				if (static_cast<int>(yintercept) == intercept_h)
				{
					//
					// the trace hit the door plane at pixel position,
					// see if the door is closed that much
					//
					// draw the door
					//

					const auto doorpos = tilehit & 0x7F;
					auto skip_draw = false;

#ifndef W3D
					if (intercept_l < 0.5)
					{
						if (intercept_l > (0.5 - (0.5 * doorposition[doorpos])))
						{
							skip_draw = true;
						}
					}
					else
					{
						if (intercept_l < ((0.5 * doorposition[doorpos]) + 0.5))
						{
							skip_draw = true;
						}
					}
#else
					if (intercept_l < doorposition[doorpos])
					{
						skip_draw = true;
					}
#endif // W3D

					if (!skip_draw)
					{
						//
						// draw the trek door
						//

						yintercept = intercept_l + get_integral(yintercept);
						xintercept = xtile + 0.5;

						HitVertDoor();

						goto nextpix;
					}
				}
			}

			//
			// midpoint is outside tile,
			// so it hit the side of the wall before a door
			//

			xt = xtile;
			yint_h = static_cast<int>(yintercept);
		}
		else
		{
			xintercept = xt;
			xtile = xt;

			yintercept = yint_h + get_fractional(yintercept);
			ytile = yint_h;

			HitVertWall();

			goto nextpix;
		}
	}

	spotvis[xs_x][xs_y] = 1;

	xt += xtilestep;

	yint = get_fractional(yintercept) + get_fractional(ystep);
	yintercept = get_integral(yintercept) + get_fractional_1(yint);

	yint_h += static_cast<int>(ystep + get_integral_1(yint));

	xs_x = xt;
	xs_y = yint_h;

	goto vertcheck;


	//
	// check intersections with horizontal walls
	//

horizcheck:

	if ((xtilestep > 0 && xint_h >= xt) || (xtilestep < 0 && xint_h <= xt))
	{
		goto vertentry;
	}

horizentry:

	if (tilemap[ys_x][ys_y] != 0)
	{
		tilehit = tilemap[ys_x][ys_y];

		if ((tilehit & 0x80) != 0)
		{
			//
			// hit a special horizontal wall, so find which coordinate a
			// door would be intersected at, and check to see if the
			// door is open past that point
			//

			xtile = xt;

			yintercept = yint_h + get_fractional(yintercept);

			if ((tilehit & 0x40) != 0)
			{
				//
				// hit a sliding horizontal wall
				//

				intercept = xstep * pwallpos;
				xint = get_fractional(intercept) + get_fractional(xintercept);
				intercept_l = get_fractional_1(xint);
				intercept_h = xint_h + static_cast<int>(intercept + get_integral_1(xint));

				if (xint_h == intercept_h)
				{
					//
					// draw the pushable wall at the new height
					//

					xintercept = intercept_h + intercept_l;
					yintercept = yt;

					HitHorizPWall();

					goto nextpix;
				}
			}
			else
			{
				intercept = 0.5 * xstep;
				xint = get_fractional(intercept) + get_fractional(xintercept);
				intercept_l = get_fractional_1(xint);
				intercept_h = xint_h + static_cast<int>(intercept + get_integral_1(xint));

				if (xint_h == intercept_h)
				{
					//
					// the trace hit the door plane at pixel position,
					// see if the door is closed that much
					//
					// draw the door
					//

					const auto doorpos = tilehit & 0x7F;

					auto skip_draw = false;

#ifndef W3D
					if (intercept_l < 0.5)
					{
						if (intercept_l > (0.5 - (0.5 * doorposition[doorpos])))
						{
							skip_draw = true;
						}
					}
					else
					{
						if (intercept_l < ((0.5 * doorposition[doorpos]) + 0.5))
						{
							skip_draw = true;
						}
					}
#else
					if (intercept_l < doorposition[doorpos])
					{
						skip_draw = true;
					}
#endif // W3D

					if (!skip_draw)
					{
						//
						// draw the trek door
						//

						xintercept = xint_h + intercept_l;
						yintercept = yt + 0.5;

						HitHorizDoor();

						goto nextpix;
					}
				}
			}


			//
			// midpoint is outside tile,
			// so it hit the side of the wall before a door
			//

			xt = xtile;
			yint_h = static_cast<int>(yintercept);
		}
		else
		{
			xintercept = xint_h + get_fractional(xintercept);
			xtile = xint_h;

			yintercept = yt;
			ytile = yt;

			HitHorizWall();

			goto nextpix;
		}
	}

	spotvis[ys_x][ys_y] = 1;

	yt += ytilestep;

	xint = get_fractional(xintercept) + get_fractional(xstep);
	xintercept = get_integral(xintercept) + get_fractional_1(xint);

	xint_h += static_cast<int>(xstep + get_integral_1(xint));

	ys_x = xint_h;
	ys_y = yt;

	goto horizcheck;


	//
	// next pixel over
	//

nextpix:
	++pixx;

	if (pixx < viewwidth)
	{
		goto pixxloop;
	}
}
