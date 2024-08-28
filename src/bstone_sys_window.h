/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A window.

#ifndef BSTONE_SYS_WINDOW_INCLUDED
#define BSTONE_SYS_WINDOW_INCLUDED

#include <climits>

#include <memory>

#include "bstone_exception.h"
#include "bstone_sys_gl_context.h"
#include "bstone_sys_renderer.h"

namespace bstone {
namespace sys {

using WindowId = unsigned int;

// ==========================================================================

constexpr auto window_min_position = -65'535;
constexpr auto window_max_position = +65'535;

// ==========================================================================

enum class WindowFullscreenType
{
	none,
	fake,
};

// ==========================================================================

enum class WindowRendererType
{
	none,
	open_gl,
};

// ==========================================================================

enum class WindowOffsetType
{
	custom,
	undefined,
	centered,
};

// ==========================================================================

class WindowOffset
{
public:
	WindowOffset() = default;

	constexpr explicit WindowOffset(int value) noexcept
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

			default: BSTONE_THROW_STATIC_SOURCE("Unknown type.");
		}
	}

	constexpr WindowOffsetType get_type() const noexcept
	{
		return type_;
	}

	constexpr int get() const noexcept
	{
		return value_;
	}

	static constexpr WindowOffset make_centered() noexcept
	{
		return WindowOffset{WindowOffsetType::centered};
	}

	static constexpr WindowOffset make_undefined() noexcept
	{
		return WindowOffset{WindowOffsetType::undefined};
	}

private:
	WindowOffsetType type_{};
	int value_{};
};

// ==========================================================================

struct WindowPosition
{
	WindowOffset x;
	WindowOffset y;
};

// ==========================================================================

struct WindowSize
{
	int width;
	int height;
};

// ==========================================================================

enum class WindowRoundedCornerType
{
	none,
	system,
	round,
	round_small,
};

// ==========================================================================

struct WindowInitParam
{
	const char* title;
	WindowOffset x;
	WindowOffset y;
	int width;
	int height;
	bool is_visible;
	WindowRoundedCornerType rounded_corner_type;
	WindowFullscreenType fullscreen_type;
	WindowRendererType renderer_type;
	const GlContextAttributes* gl_attributes;
};

// ==========================================================================

class Window
{
public:
	Window();
	virtual ~Window();

	const char* get_title();
	void set_title(const char* title);

	WindowPosition get_position();
	void set_position(WindowPosition position);
	void center();

	WindowSize get_size();
	void set_size(WindowSize size);

	void show(bool is_visible);

	void set_rounded_corner_type(WindowRoundedCornerType value);

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

	virtual void do_set_rounded_corner_type(WindowRoundedCornerType value) = 0;

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
