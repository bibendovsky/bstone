/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Planet Strike fizzle effect.
//


#include "bstone_ps_fizzle_fx.h"

#include "3d_def.h"
#include "id_heads.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "gfxv.h"
#include "bstone_version.h"


void CA_CacheScreen(
	std::int16_t chunk);


namespace bstone
{


PsFizzleFX::PsFizzleFX()
	:
	buffer_(vga_ref_width * vga_ref_height)
{
}

PsFizzleFX::PsFizzleFX(
	PsFizzleFX&& rhs) noexcept
	:
	buffer_{std::move(rhs.buffer_)}
{
}

PsFizzleFX::~PsFizzleFX()
{
	uninitialize();
}

void PsFizzleFX::initialize(bool has_vanilla_appearence)
{
	has_vanilla_appearence_ = has_vanilla_appearence;

	const auto version_padding = 1;

	VgaBuffer current_screen;
	vid_export_ui(current_screen);

	CA_CacheScreen(TITLE2PIC);
	fontnumber = 2;
	PrintX = WindowX + version_padding;
	PrintY = WindowY + version_padding;
	VWB_Bar(WindowX, WindowY, WindowW, WindowH, VERSION_TEXT_BKCOLOR);

	SETFONTCOLOR(VERSION_TEXT_COLOR, VERSION_TEXT_BKCOLOR);
	US_Print(bstone::get_version().string_short);

	vid_export_ui(buffer_);
	vid_import_ui(current_screen);
}

void PsFizzleFX::uninitialize()
{
}

bool PsFizzleFX::is_abortable() const
{
	return true;
}

bool PsFizzleFX::is_vanilla_only() const
{
	return true;
}

int PsFizzleFX::get_frame_count() const
{
	return 70;
}

int PsFizzleFX::get_y() const
{
	return 0;
}

int PsFizzleFX::get_height() const
{
	return vga_ref_height;
}

void PsFizzleFX::plot(
	const int x,
	const int y)
{
	const auto offset = (y * vga_ref_width) + x;
	const auto color_index = buffer_[offset];

	VL_Plot(x, y, color_index);
}

void PsFizzleFX::skip_to_the_end()
{
	vid_import_ui(buffer_);
}

bool PsFizzleFX::has_vanilla_appearence() const
{
	return has_vanilla_appearence_;
}

} // bstone
