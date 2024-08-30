/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Reference values.
//


#ifndef BSTONE_REF_VALUES_INCLUDED
#define BSTONE_REF_VALUES_INCLUDED


const int vga_ref_width = 320;
const int vga_ref_height = 200;
const int vga_ref_height_4x3 = (vga_ref_height * 12) / 10;

const int ref_top_bar_height = 16;
const int ref_bottom_bar_height = 48;

#ifdef __vita__
const int ref_3d_margin = 0;
#else
const int ref_3d_margin = 3;
#endif

const int ref_center_x = (vga_ref_width / 2) - 1;

const int ref_view_top_y = ref_top_bar_height;
const int ref_view_bottom_y = vga_ref_height - ref_bottom_bar_height - 1;

const int ref_view_height = vga_ref_height - (ref_top_bar_height + ref_bottom_bar_height);

const int ref_3d_view_height = ref_view_height - (2 * ref_3d_margin);

const int ref_3d_view_top_y = ref_top_bar_height + ref_3d_margin;
const int ref_3d_view_bottom_y = vga_ref_height - (ref_bottom_bar_height + ref_3d_margin) - 1;


#endif // !BSTONE_REF_VALUES_INCLUDED
