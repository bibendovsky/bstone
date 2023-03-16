/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_RENDERER_INCLUDED)
#define BSTONE_SYS_RENDERER_INCLUDED

#include <memory>
#include "bstone_span.h"
#include "bstone_sys_color.h"
#include "bstone_sys_r2_rect.h"
#include "bstone_sys_pixel_format.h"
#include "bstone_sys_texture.h"

namespace bstone {
namespace sys {

struct RendererInitParam
{
	bool is_vsync;
};

// ==========================================================================

struct RendererViewport : public R2Rect
{
	using R2Rect::R2Rect;
};

// ==========================================================================

class Renderer
{
public:
	Renderer() = default;
	virtual ~Renderer() = default;

	const char* get_name() const;

	void set_viewport();
	void set_viewport(const RendererViewport& viewport);

	void clear();
	void set_draw_color(Color color);
	void fill(Span<const R2Rect> rects);
	void present();

	void read_pixels(PixelFormat pixel_format, void* pixels, int pitch);

	TextureUPtr make_texture(const TextureInitParam& param);

private:
	virtual const char* do_get_name() const = 0;

	virtual void do_set_viewport(const RendererViewport* viewport) = 0;

	virtual void do_clear() = 0;
	virtual void do_set_draw_color(Color color) = 0;
	virtual void do_fill(Span<const R2Rect> rects) = 0;
	virtual void do_present() = 0;

	virtual void do_read_pixels(const R2Rect* rect, PixelFormat pixel_format, void* pixels, int pitch) = 0;

	virtual TextureUPtr do_make_texture(const TextureInitParam& param) = 0;
};

// ==========================================================================

using RendererUPtr = std::unique_ptr<Renderer>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_RENDERER_INCLUDED
