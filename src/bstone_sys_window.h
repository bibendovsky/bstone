/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_WINDOW_INCLUDED)
#define BSTONE_SYS_WINDOW_INCLUDED

#include <climits>
#include <memory>
#include "bstone_enum_flags.h"
#include "bstone_sys_gl_context.h"
#include "bstone_sys_r2_extent.h"
#include "bstone_sys_r2_offset.h"
#include "bstone_sys_renderer.h"

namespace bstone {
namespace sys {

using WindowId = unsigned int;

// ==========================================================================

constexpr auto window_min_position = -65'535;
constexpr auto window_max_position = +65'535;

constexpr auto window_position_centered = INT_MAX - 0;
constexpr auto window_position_undefined = INT_MAX - 1;

// ==========================================================================

struct WindowPosition : public R2Offset
{
	using R2Offset::R2Offset;
};

struct WindowSize : public R2Extent
{
	using R2Extent::R2Extent;
};

// ==========================================================================

struct WindowInitParam
{
	const char* title{};
	int x{};
	int y{};
	int width{};
	int height{};
	bool is_opengl{};
	bool is_visible{};
	bool is_fake_fullscreen{};
	const GlContextAttributes* gl_attributes{};
};

// ==========================================================================

class Window
{
public:
	Window() = default;
	virtual ~Window() = default;

	const char* get_title();
	void set_title(const char* title);

	WindowPosition get_position();
	void set_position(WindowPosition position);
	void center();

	WindowSize get_size();
	void set_size(WindowSize size);

	void show(bool is_visible);

	bool is_fake_fullscreen();
	void set_fake_fullscreen(bool is_fake_fullscreen);

	GlContextUPtr make_gl_context();

	WindowSize gl_get_drawable_size();
	void gl_swap_buffers();

	RendererUPtr make_renderer(const RendererInitParam& param);

private:
	virtual const char* do_get_title() = 0;
	virtual void do_set_title(const char* title) = 0;

	virtual WindowPosition do_get_position() = 0;
	virtual void do_set_position(WindowPosition position) = 0;

	virtual WindowSize do_get_size() = 0;
	virtual void do_set_size(WindowSize size) = 0;

	virtual void do_show(bool is_visible) = 0;

	virtual bool do_is_fake_fullscreen() = 0;
	virtual void do_set_fake_fullscreen(bool is_fake_fullscreen) = 0;

	virtual GlContextUPtr do_make_gl_context() = 0;

	virtual WindowSize do_gl_get_drawable_size() = 0;
	virtual void do_gl_swap_buffers() = 0;

	virtual RendererUPtr do_make_renderer(const RendererInitParam& param) = 0;
};

// ==========================================================================

using WindowUPtr = std::unique_ptr<Window>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_WINDOW_INCLUDED
