/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Fizzle effect (base class).
//


#include "bstone_fizzle_fx.h"

#include "3d_def.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_vl.h"

#include "bstone_exception.h"


void IN_StartAck();
bool IN_CheckAck();


namespace bstone
{


FizzleFX::FizzleFX() noexcept = default;

FizzleFX::FizzleFX(FizzleFX&& rhs) = default;

FizzleFX::~FizzleFX() = default;

bool FizzleFX::present()
{
	if (!has_vanilla_appearence() && !is_vanilla_only())
	{
		vid_hw_enable_fizzle_fx(true);
		vid_hw_set_fizzle_fx_ratio(0.0F);
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

			if (!has_vanilla_appearence() && !is_vanilla_only())
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

	if (!has_vanilla_appearence() && !is_vanilla_only())
	{
		vid_hw_enable_fizzle_fx(false);
	}

	return is_aborted;
}

bool FizzleFX::is_abortable() const
try {
	fail_not_implemented();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool FizzleFX::is_vanilla_only() const
try {
	fail_not_implemented();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int FizzleFX::get_frame_count() const
try {
	fail_not_implemented();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int FizzleFX::get_y() const
try {
	fail_not_implemented();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int FizzleFX::get_height() const
try {
	fail_not_implemented();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void FizzleFX::plot(
	const int x,
	const int y)
try {
	static_cast<void>(x);
	static_cast<void>(y);

	fail_not_implemented();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void FizzleFX::skip_to_the_end()
try {
	fail_not_implemented();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void FizzleFX::fail_not_implemented()
{
	BSTONE_THROW_STATIC_SOURCE("Not implemented.");
}


} // bstone
