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
// Fizzle effect (base class).
//


#include "bstone_fizzle_fx.h"

#include "3d_def.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_vl.h"


void IN_StartAck();
bool IN_CheckAck();


namespace bstone
{


bool FizzleFX::present(
	const bool trigger_fade)
{
	if (vid_is_hw_ && !is_vanilla_only())
	{
		vid_hw_enable_fizzle_fx(true);
		vid_hw_set_fizzle_fx_ratio(0.0F);
	}

	if (trigger_fade)
	{
		vid_is_fizzle_fade = true;
	}

	const auto y_offset = get_y();
	const auto width = vga_ref_width;
	const auto height = get_height();
	const auto frame_count = get_frame_count();
	const auto area = width * height;

	auto rndval = 1;
	const auto pixels_per_frame = area / frame_count;
	auto remain_pixels = area % frame_count;
	auto frame = 0;

	IN_StartAck();

	TimeCount = 0;
	LastScan = ScanCode::sc_none;

	auto is_finished = false;
	auto is_aborted = false;
	auto do_full_copy = false;

	while (!is_finished)
	{
		if (is_abortable() && IN_CheckAck())
		{
			is_aborted = true;
			do_full_copy = true;
		}

		if (!do_full_copy)
		{
			const auto pixel_count = pixels_per_frame + remain_pixels;

			remain_pixels = 0;

			if (vid_is_hw_ && !is_vanilla_only())
			{
				if ((frame + 1) >= frame_count)
				{
					do_full_copy = true;
				}
				else
				{
					const auto ratio = static_cast<float>(frame) / static_cast<float>(frame_count);

					vid_hw_set_fizzle_fx_ratio(ratio);
				}
			}
			else
			{
				for (auto p = 0; p < pixel_count; ++p)
				{
					auto x = (rndval >> 8) & 0xFFFF;
					auto y = ((rndval & 0xFF) - 1) & 0xFF;

					auto carry = ((rndval & 1) != 0);

					rndval >>= 1;

					if (carry)
					{
						rndval ^= 0x00012000;
					}

					if (x >= width || y >= height)
					{
						continue;
					}

					plot(x, y_offset + y);

					if (rndval == 1)
					{
						do_full_copy = true;
					}
				}
			}
		}
		else
		{
			is_finished = true;

			skip_to_the_end();
		}

		VL_RefreshScreen();

		++frame;

		CalcTics();
	}

	if (trigger_fade)
	{
		vid_is_fizzle_fade = false;
	}

	if (vid_is_hw_ && !is_vanilla_only())
	{
		vid_hw_enable_fizzle_fx(false);
	}

	return is_aborted;
}

bool FizzleFX::is_abortable() const
{
	throw "Not implemented.";
}

bool FizzleFX::is_vanilla_only() const
{
	throw "Not implemented.";
}

int FizzleFX::get_frame_count() const
{
	throw "Not implemented.";
}

int FizzleFX::get_y() const
{
	throw "Not implemented.";
}

int FizzleFX::get_height() const
{
	throw "Not implemented.";
}

void FizzleFX::plot(
	const int x,
	const int y)
{
	static_cast<void>(x);
	static_cast<void>(y);

	throw "Not implemented.";
}

void FizzleFX::skip_to_the_end()
{
	throw "Not implemented.";
}


} // bstone
