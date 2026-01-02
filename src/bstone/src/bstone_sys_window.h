/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window

#ifndef BSTONE_SYS_WINDOW_INCLUDED
#define BSTONE_SYS_WINDOW_INCLUDED

#include "bstone_exception.h"
#include "bstone_sys_display_mode.h"
#include "bstone_sys_gl_context.h"
#include "bstone_sys_renderer.h"
#include <memory>

namespace bstone::sys {

using WindowId = unsigned int;

// ======================================

constexpr int window_min_position = -65'535;
constexpr int window_max_position = +65'535;

// ======================================

enum class WindowFullscreenType
{
	none,
	exclusive,
	fake,
};

enum class WindowRendererType
{
	none,
	open_gl,
	vulkan,
};

enum class WindowOffsetType
{
	custom,
	undefined,
	centered,
};

// ======================================

class WindowOffset
{
public:
	WindowOffset() = default;

	constexpr explicit WindowOffset(int value)
		:
		type_{WindowOffsetType::custom},
		value_{value}
	{}

	constexpr explicit WindowOffset(WindowOffsetType type)
	{
		switch (type)
		{
			case WindowOffsetType::undefined:
			case WindowOffsetType::centered:
				type_ = type;
				break;
			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown type.");
		}
	}

	constexpr WindowOffsetType get_type() const
	{
		return type_;
	}

	constexpr int get() const
	{
		return value_;
	}

	static constexpr WindowOffset make_centered()
	{
		return WindowOffset{WindowOffsetType::centered};
	}

	static constexpr WindowOffset make_undefined()
	{
		return WindowOffset{WindowOffsetType::undefined};
	}

private:
	WindowOffsetType type_{};
	int value_{};
};

// ======================================

struct WindowPosition
{
	WindowOffset x;
	WindowOffset y;
};

struct WindowSize
{
	int width;
	int height;
};

enum class WindowRoundedCornerType
{
	none,
	system,
	round,
	round_small,
};

struct WindowInitParam
{
	const char* title;
	WindowOffset x;
	WindowOffset y;
	int width;
	int height;
	bool is_visible;
	WindowRoundedCornerType rounded_corner_type;
	WindowRendererType renderer_type;
	const GlContextAttributes* gl_attributes;
};

// ======================================

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
	DisplayMode get_display_mode();
	void set_display_mode(const DisplayMode& display_mode);
	void show(bool is_visible);
	void set_rounded_corner_type(WindowRoundedCornerType value);
	WindowFullscreenType get_fullscreen_mode();
	void set_fullscreen_mode(WindowFullscreenType fullscreen_mode);
	GlContextUPtr gl_make_context();
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
	virtual DisplayMode do_get_display_mode() = 0;
	virtual void do_set_display_mode(const DisplayMode& display_mode) = 0;
	virtual void do_show(bool is_visible) = 0;
	virtual void do_set_rounded_corner_type(WindowRoundedCornerType value) = 0;
	virtual WindowFullscreenType do_get_fullscreen_mode() = 0;
	virtual void do_set_fullscreen_mode(WindowFullscreenType fullscreen_mode) = 0;
	virtual GlContextUPtr do_gl_make_context() = 0;
	virtual WindowSize do_gl_get_drawable_size() = 0;
	virtual void do_gl_swap_buffers() = 0;
	virtual RendererUPtr do_make_renderer(const RendererInitParam& param) = 0;
};

// ======================================

using WindowUPtr = std::unique_ptr<Window>;

} // namespace bstone::sys

#endif // BSTONE_SYS_WINDOW_INCLUDED
