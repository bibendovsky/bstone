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
// Generic fizzle effect.
//


#include "bstone_generic_fizzle_fx.h"
#include "3d_def.h"
#include "id_vl.h"


namespace bstone
{


GenericFizzleFX::GenericFizzleFX(
    uint8_t plot_color,
    bool is_transparent) :
        plot_color_(plot_color),
        is_transparent_(is_transparent),
        y_offset_(),
        height_()
{
}

GenericFizzleFX::~GenericFizzleFX()
{
    uninitialize();
}

void GenericFizzleFX::initialize()
{
    const auto full_height = ::vga_ref_height - TOP_STRIP_HEIGHT - STATUSLINES;

    height_ = ::viewheight;

    y_offset_ = TOP_STRIP_HEIGHT + ((full_height - ::viewheight) / 2);

    ::VL_Bar(
        0,
        y_offset_,
        ::vga_ref_width,
        height_,
        plot_color_,
        is_transparent_);
}

void GenericFizzleFX::uninitialize()
{
}

bool GenericFizzleFX::is_abortable() const
{
    return false;
}

int GenericFizzleFX::get_frame_count() const
{
    return 70;
}

int GenericFizzleFX::get_y() const
{
    return y_offset_;
}

int GenericFizzleFX::get_height() const
{
    return height_;
}

void GenericFizzleFX::plot(
    int x,
    int y)
{
    ::VL_Plot(
        x,
        y,
        plot_color_,
        !is_transparent_);
}

void GenericFizzleFX::skip_to_the_end()
{
    ::VL_Bar(
        0,
        get_y(),
        ::vga_ref_width,
        get_height(),
        plot_color_,
        !is_transparent_);
}


} // bstone
