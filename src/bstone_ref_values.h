#ifndef BSTONE_REF_VALUES_INCLUDED
#define BSTONE_REF_VALUES_INCLUDED


const int vga_ref_width = 320;
const int vga_ref_height = 200;
const int vga_ref_height_4x3 = (::vga_ref_height * 12) / 10;

const int ref_top_bar_height = 16;
const int ref_bottom_bar_height = 48;
const int ref_3d_margin = 3;

const int ref_center_x = (::vga_ref_width / 2) - 1;

const int ref_view_top = ::ref_top_bar_height;
const int ref_view_bottom = ::vga_ref_height - ref_bottom_bar_height;

const int ref_view_height =
    ::vga_ref_height -
    ::ref_top_bar_height -
    ::ref_bottom_bar_height;

const int ref_3d_view_height =
    ::ref_view_height -
    (2 * ::ref_3d_margin);

const int ref_3d_view_top = ::ref_top_bar_height + ::ref_3d_margin;
const int ref_3d_view_bottom = ::ref_3d_view_top + ::ref_3d_view_height;


#endif // BSTONE_REF_VALUES_INCLUDED
