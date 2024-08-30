/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 2D renderer.

#include "bstone_sys_renderer.h"

namespace bstone {
namespace sys {

Renderer::Renderer() = default;

Renderer::~Renderer() = default;

const char* Renderer::get_name() const
{
	return do_get_name();
}

void Renderer::set_viewport()
{
	do_set_viewport(nullptr);
}

void Renderer::clear()
{
	do_clear();
}

void Renderer::set_draw_color(Color color)
{
	do_set_draw_color(color);
}

void Renderer::fill(Span<const Rectangle> rectangles)
{
	do_fill(rectangles);
}

void Renderer::present()
{
	do_present();
}

void Renderer::read_pixels(PixelFormat pixel_format, void* pixels, int pitch)
{
	do_read_pixels(nullptr, pixel_format, pixels, pitch);
}

TextureUPtr Renderer::make_texture(const TextureInitParam& param)
{
	return do_make_texture(param);
}

} // namespace sys
} // namespace bstone
