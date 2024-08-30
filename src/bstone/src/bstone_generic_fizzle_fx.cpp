/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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
	bool has_vanilla_appearence_{};
	std::uint8_t plot_color_{};
	bool is_transparent_{};
	int y_offset_{};
	int height_{};
	VgaBuffer old_ui_{};
	UiMaskBuffer old_ui_mask_{};
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
	GenericFizzleFX&& rhs) noexcept
	:
	impl_{std::move(rhs.impl_)}
{
}

GenericFizzleFX::~GenericFizzleFX()
{
	uninitialize();
}

void GenericFizzleFX::initialize(bool has_vanilla_appearence)
{
	impl_->has_vanilla_appearence_ = has_vanilla_appearence;

	if (!has_vanilla_appearence && !is_vanilla_only())
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
	if (!has_vanilla_appearence() && !is_vanilla_only())
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
	if (!has_vanilla_appearence() && !is_vanilla_only())
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

bool GenericFizzleFX::has_vanilla_appearence() const
{
	return impl_->has_vanilla_appearence_;
}

} // bstone
