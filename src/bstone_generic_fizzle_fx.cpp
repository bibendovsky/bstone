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
// Generic fizzle effect.
//


#include "bstone_generic_fizzle_fx.h"

#include "id_vl.h"


namespace bstone
{


class GenericFizzleFX::Impl
{
public:
	std::uint8_t plot_color_;
	bool is_transparent_;
	int y_offset_;
	int height_;
	VgaBuffer old_ui_;
	UiMaskBuffer old_ui_mask_;


	Impl() :
		plot_color_{},
		is_transparent_{},
		y_offset_{},
		height_{},
		old_ui_{},
		old_ui_mask_{}
	{
	}
}; // Impl


GenericFizzleFX::GenericFizzleFX(
	const std::uint8_t plot_color,
	const bool is_transparent)
	:
	impl_{std::make_unique<Impl>()}
{
	impl_->plot_color_ = plot_color;
	impl_->is_transparent_ = is_transparent;
}

GenericFizzleFX::GenericFizzleFX(
	GenericFizzleFX&& rhs)
	:
	impl_{std::move(rhs.impl_)}
{
}

GenericFizzleFX::~GenericFizzleFX()
{
	uninitialize();
}

void GenericFizzleFX::initialize()
{
	if (vid_is_hw_ && !is_vanilla_only())
	{
		vid_hw_set_fizzle_fx_color_index(impl_->plot_color_);
		vid_hw_enable_fizzle_fx_fading(impl_->is_transparent_);

		return;
	}

	impl_->y_offset_ = ref_view_top_y;
	impl_->height_ = ref_view_height;

	if (!impl_->is_transparent_)
	{
		vid_export_ui(impl_->old_ui_);
		vid_export_ui_mask(impl_->old_ui_mask_);

		VL_Bar(0, impl_->y_offset_, vga_ref_width, impl_->height_, impl_->plot_color_, false);
	}
}

void GenericFizzleFX::uninitialize()
{
}

bool GenericFizzleFX::is_abortable() const
{
	return false;
}

bool GenericFizzleFX::is_vanilla_only() const
{
	return false;
}

int GenericFizzleFX::get_frame_count() const
{
	return 70;
}

int GenericFizzleFX::get_y() const
{
	return impl_->y_offset_;
}

int GenericFizzleFX::get_height() const
{
	return impl_->height_;
}

void GenericFizzleFX::plot(
	const int x,
	const int y)
{
	if (vid_is_hw_ && !is_vanilla_only())
	{
		return;
	}

	if (impl_->is_transparent_)
	{
		VL_Plot(x, y, impl_->plot_color_, !impl_->is_transparent_);
	}
	else
	{
		const auto index = (y * vga_ref_width) + x;

		VL_Plot(x, y, impl_->old_ui_[index], !impl_->old_ui_mask_[index]);
	}
}

void GenericFizzleFX::skip_to_the_end()
{
	if (vid_is_hw_ && !is_vanilla_only())
	{
		vid_hw_set_fizzle_fx_ratio(1.0F);

		return;
	}

	if (impl_->is_transparent_)
	{
		VL_Bar(0, get_y(), vga_ref_width, get_height(), impl_->plot_color_, !impl_->is_transparent_);
	}
	else
	{
		vid_import_ui(impl_->old_ui_);
		vid_import_ui_mask(impl_->old_ui_mask_);
	}
}


} // bstone
