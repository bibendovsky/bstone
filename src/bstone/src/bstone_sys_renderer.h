/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 2D renderer

#ifndef BSTONE_SYS_RENDERER_INCLUDED
#define BSTONE_SYS_RENDERER_INCLUDED

#include "bstone_sys_color.h"
#include "bstone_sys_pixel_format.h"
#include "bstone_sys_rectangle.h"
#include "bstone_sys_texture.h"
#include <memory>
#include <span>

namespace bstone::sys {

struct RendererInitParam
{
	bool is_vsync;
};

// ======================================

struct RendererViewport : public Rect
{
	using Rect::Rect;
};

// ======================================

class Renderer
{
public:
	Renderer() = default;
	virtual ~Renderer() = default;

	virtual const char* get_name() const = 0;
	virtual void set_viewport() = 0;
	virtual void clear() = 0;
	virtual void set_draw_color(Color color) = 0;
	virtual void fill(std::span<const FRect> rects) = 0;
	virtual void present() = 0;
	virtual void read_pixels(PixelFormat pixel_format, void* pixels, int pitch) = 0;
	virtual TextureUPtr make_texture(const TextureInitParam& param) = 0;
};

// ======================================

using RendererUPtr = std::unique_ptr<Renderer>;

} // namespace bstone::sys

#endif // BSTONE_SYS_RENDERER_INCLUDED
