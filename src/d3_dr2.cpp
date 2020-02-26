/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


extern int finetangent[FINEANGLES / 4];
extern int viewwidth;
extern std::uint8_t tilemap[MAPSIZE][MAPSIZE];
extern std::uint8_t spotvis[MAPSIZE][MAPSIZE];
extern std::vector<int> pixelangle;
extern int midangle;
extern int focaltx;
extern int focalty;
extern int viewtx;
extern int viewty;
extern int viewx;
extern int viewy;
extern int xpartialup;
extern int xpartialdown;
extern int ypartialup;
extern int ypartialdown;
extern int tilehit;
extern int pixx;
extern WallHeight wallheight;
extern int xtile;
extern int ytile;
extern int xtilestep;
extern int ytilestep;
extern int xintercept;
extern int yintercept;
extern int xstep;
extern int ystep;
extern std::uint16_t doorposition[MAXDOORS];
extern std::uint16_t pwallpos;


static int partial_by_step(
	int step,
	int partial)
{
	int fracs = ((static_cast<unsigned int>(step) & 0xFFFF) * partial) >> 16;
	int ints = (step >> 16) * partial;
	return ints + fracs + (step < 0 ? 1 : 0);
}


void AsmRefresh()
{
	int h_op;
	int v_op;
	int xpartial;
	int ypartial;
	int doorpos;
	int intercept;
	int intercept_h; // high part of intercept
	int intercept_l; // low part of intercept
	int xs; // temporary xspot
	int xt; // temporary xtile
	int xint; // temporary xintercept
	int xint_h; // high part of temporary xintercept
	int ys; // temporary yspot
	int yt; // temporary ytile
	int yint; // temporary yintercept
	int yint_h; // high part of temporary yintercept

	::vid_hw_clear_wall_render_list();
	::vid_hw_clear_pushwall_render_list();
	::vid_hw_clear_door_render_list();

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

		h_op = 1;
		v_op = 1;

		xstep = finetangent[angle - DEG270];
		ystep = finetangent[DEG360 - 1 - angle];

		xpartial = xpartialup;
		ypartial = ypartialup;
	}
	else if (angle >= DEG180)
	{
		xtilestep = -1;
		ytilestep = 1;

		h_op = -1;
		v_op = 1;

		xstep = -finetangent[DEG270 - 1 - angle];
		ystep = finetangent[angle - DEG180];

		xpartial = xpartialdown;
		ypartial = ypartialup;
	}
	else if (angle >= DEG90)
	{
		xtilestep = -1;
		ytilestep = -1;

		h_op = -1;
		v_op = -1;

		xstep = -finetangent[angle - DEG90];
		ystep = -finetangent[DEG180 - 1 - angle];

		xpartial = xpartialdown;
		ypartial = ypartialdown;
	}
	else
	{
		xtilestep = 1;
		ytilestep = -1;

		h_op = 1;
		v_op = -1;

		xstep = finetangent[DEG90 - 1 - angle];
		ystep = -finetangent[angle];

		xpartial = xpartialup;
		ypartial = ypartialdown;
	}


	//
	// initialise variables for intersection testing
	//

	yintercept = viewy + partial_by_step(ystep, xpartial);
	yint_h = yintercept >> 16;

	xtile = focaltx + xtilestep;
	xs = (xtile << 6) + yint_h;

	xint = viewx + partial_by_step(xstep, ypartial);
	xint_h = xint >> 16;
	xintercept &= 0xFFFF0000;
	xintercept |= xint & 0xFFFF;

	yt = focalty + ytilestep;
	ys = (xint_h << 6) + yt;

	xt = xtile;
	yint_h = yintercept >> 16;


	//
	// trace along this angle until we hit a wall
	//
	// CORE LOOP!
	//

	//
	// check intersections with vertical walls
	//

vertcheck:

	if ((v_op < 0 && yint_h <= yt) || (v_op > 0 && yint_h >= yt))
	{
		goto horizentry;
	}

vertentry:

	if (reinterpret_cast<const std::uint8_t*>(tilemap)[xs] != 0)
	{
		tilehit &= 0xFF00;
		tilehit |= reinterpret_cast<const std::uint8_t*>(tilemap)[xs];

		if ((tilehit & 0x80) != 0)
		{
			//
			// hit a special vertical wall, so find which coordinate a
			// door would be intersected at, and check to see if the
			// door is open past that point
			//

			xtile = xt;

			yintercept &= 0xFFFF;
			yintercept |= yint_h << 16;

			if ((tilehit & 0x40) != 0)
			{
				//
				// hit a sliding vertical wall
				//

				intercept = (ystep * pwallpos) >> 6;
				yint = (intercept & 0xFFFF) + (yintercept & 0xFFFF);
				intercept_l = yint & 0xFFFF;
				intercept_h = (intercept >> 16) + yint_h + (yint >> 16);

				if (yint_h == intercept_h)
				{
					//
					// draw the pushable wall at the new height
					//

					yintercept = (intercept_h << 16) | intercept_l;
					xintercept = xt << 16;

					HitVertPWall();

					goto nextpix;
				}
			}
			else
			{
				doorpos = tilehit & 0x7F;

				intercept = yintercept + (ystep >> 1);
				intercept_l = intercept & 0xFFFF;
				intercept_h = intercept >> 16;

				if ((yintercept >> 16) == intercept_h)
				{
					//
					// the trace hit the door plane at pixel position,
					// see if the door is closed that much
					//
					// draw the door
					//

					bool skip_draw = false;

#ifndef W3D
					if (intercept_l < 0x8000)
					{
						if (intercept_l >
							(0x7FFF - (doorposition[doorpos] >> 1)))
						{
							skip_draw = true;
						}
					}
					else
					{
						if (intercept_l <
							((doorposition[doorpos] >> 1) + 0x8000))
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

						yintercept &= 0xFFFF0000;
						yintercept |= intercept_l;

						xintercept = (xtile << 16) | 0x8000;

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
			yint_h = yintercept >> 16;
		}
		else
		{
			xintercept = xt << 16;
			xtile = xt;

			yintercept &= 0xFFFF;
			yintercept |= yint_h << 16;
			ytile = yint_h;

			HitVertWall();

			goto nextpix;
		}
	}

	reinterpret_cast<std::uint8_t*>(spotvis)[xs] = 1;

	xt += xtilestep;

	yint = (yintercept & 0xFFFF) + (ystep & 0xFFFF);
	yintercept &= 0xFFFF0000;
	yintercept |= yint & 0xFFFF;

	yint_h += (ystep >> 16) + (yint >> 16);

	xs = (xt << 6) + yint_h;

	goto vertcheck;



	//
	// check intersections with horizontal walls
	//

horizcheck:

	if ((h_op > 0 && xint_h >= xt) || (h_op < 0 && xint_h <= xt))
	{
		goto vertentry;
	}

horizentry:

	if (reinterpret_cast<const std::uint8_t*>(tilemap)[ys] != 0)
	{
		tilehit &= 0xFF00;
		tilehit |= reinterpret_cast<const std::uint8_t*>(tilemap)[ys];

		if ((tilehit & 0x80) != 0)
		{
			//
			// hit a special horizontal wall, so find which coordinate a
			// door would be intersected at, and check to see if the
			// door is open past that point
			//

			xtile = xt;

			yintercept &= 0xFFFF;
			yintercept |= yint_h << 16;

			if ((tilehit & 0x40) != 0)
			{
				//
				// hit a sliding horizontal wall
				//

				intercept = (xstep * pwallpos) >> 6;
				xint = (intercept & 0xFFFF) + (xintercept & 0xFFFF);
				intercept_l = xint & 0xFFFF;
				intercept_h = (intercept >> 16) + xint_h + (xint >> 16);

				if (xint_h == intercept_h)
				{
					//
					// draw the pushable wall at the new height
					//

					xintercept = (intercept_h << 16) | intercept_l;
					yintercept = yt << 16;

					HitHorizPWall();

					goto nextpix;
				}
			}
			else
			{
				doorpos = tilehit & 0x7F;

				intercept = xstep >> 1;
				xint = (intercept & 0xFFFF) + (xintercept & 0xFFFF);
				intercept_l = xint & 0xFFFF;
				intercept_h = (intercept >> 16) + xint_h + (xint >> 16);

				if (xint_h == intercept_h)
				{
					//
					// the trace hit the door plane at pixel position,
					// see if the door is closed that much
					//
					// draw the door
					//

					bool skip_draw = false;

#ifndef W3D
					if (intercept_l < 0x8000)
					{
						if (intercept_l >
							(0x7FFF - (doorposition[doorpos] >> 1)))
						{
							skip_draw = true;
						}
					}
					else
					{
						if (intercept_l <
							((doorposition[doorpos] >> 1) + 0x8000))
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

						xintercept = (xint_h << 16) | intercept_l;
						yintercept = (yt << 16) | 0x8000;

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
			yint_h = yintercept >> 16;
		}
		else
		{
			xintercept &= 0xFFFF;
			xintercept |= xint_h << 16;
			xtile = xint_h;

			yintercept = yt << 16;
			ytile = yt;

			HitHorizWall();

			goto nextpix;
		}
	}

	reinterpret_cast<std::uint8_t*>(spotvis)[ys] = 1;

	yt += ytilestep;

	xint = (xintercept & 0xFFFF) + (xstep & 0xFFFF);
	xintercept &= 0xFFFF0000;
	xintercept |= xint & 0xFFFF;
	xint_h += (xstep >> 16) + (xint >> 16);

	ys = (xint_h << 6) + yt;

	goto horizcheck;


	//
	// next pixel over
	//

nextpix:
	++pixx;

	if (pixx < ::viewwidth)
	{
		goto pixxloop;
	}
}
