/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// Planet Strike fizzle effect.
//


#include "bstone_ps_fizzle_fx.h"
#include "3d_def.h"
#include "id_vl.h"


void CA_CacheScreen(
    int16_t chunk);


namespace bstone
{


PSFizzleFX::PSFizzleFX() :
        buffer_(::vga_ref_width * ::vga_ref_height)
{
}

PSFizzleFX::~PSFizzleFX()
{
    uninitialize();
}

void PSFizzleFX::initialize()
{
    const auto version_padding = 1;

    VgaBuffer current_screen;

    ::vid_export_ui(current_screen);


    ::CA_CacheScreen(TITLE2PIC);
    ::fontnumber = 2;
    ::PrintX = ::WindowX + version_padding;
    ::PrintY = ::WindowY + version_padding;

    ::VWB_Bar(
        ::WindowX,
        ::WindowY,
        ::WindowW,
        ::WindowH,
        VERSION_TEXT_BKCOLOR);

    SETFONTCOLOR(
        VERSION_TEXT_COLOR,
        VERSION_TEXT_BKCOLOR);

    ::US_Print(::get_version_string().c_str());

    ::vid_export_ui(buffer_);

    ::vid_import_ui(current_screen);
}

void PSFizzleFX::uninitialize()
{
}

bool PSFizzleFX::is_abortable() const
{
    return true;
}

int PSFizzleFX::get_frame_count() const
{
    return 70;
}

int PSFizzleFX::get_y() const
{
    return 0;
}

int PSFizzleFX::get_height() const
{
    return ::vga_ref_height;
}

void PSFizzleFX::plot(
    int x,
    int y)
{
    const auto offset = (y * ::vga_ref_width) + x;
    const auto color_index = buffer_[offset];

    ::VL_Plot(
        x,
        y,
        color_index);
}

void PSFizzleFX::skip_to_the_end()
{
    ::vid_import_ui(buffer_);
}


} // bstone
