/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sdl_exception.h"
#include "bstone_sys_sdl_window_utils.h"

namespace bstone {
namespace sys {
namespace detail {

namespace {

int sdl_make_window_position_coord(int position, WindowInitPositionType position_type)
try
{
	switch (position_type)
	{
		case WindowInitPositionType::undefined: return SDL_WINDOWPOS_UNDEFINED;
		case WindowInitPositionType::custom: return position;
		case WindowInitPositionType::centered: return SDL_WINDOWPOS_CENTERED;
		default: BSTONE_STATIC_THROW("Unknown position type.");
	}
}
BSTONE_FUNC_STATIC_THROW_NESTED

Uint32 sdl_map_window_flags(WindowFlags window_flags) noexcept
{
	auto sdl_flags = Uint32{};

	if ((window_flags & WindowFlags::high_dpi) != WindowFlags{})
	{
		sdl_flags |= SDL_WINDOW_ALLOW_HIGHDPI;
	}

	if ((window_flags & WindowFlags::shown) != WindowFlags{})
	{
		sdl_flags |= SDL_WINDOW_SHOWN;
	}

	if ((window_flags & WindowFlags::hidden) != WindowFlags{})
	{
		sdl_flags |= SDL_WINDOW_HIDDEN;
	}

	if ((window_flags & WindowFlags::fullscreen_desktop) != WindowFlags{})
	{
		sdl_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	if ((window_flags & WindowFlags::borderless) != WindowFlags{})
	{
		sdl_flags |= SDL_WINDOW_BORDERLESS;
	}

	if ((window_flags & WindowFlags::opengl) != WindowFlags{})
	{
		sdl_flags |= SDL_WINDOW_OPENGL;
	}

	return sdl_flags;
}

WindowFlags sdl_map_window_flags(Uint32 sdl_window_flags) noexcept
{
	auto flags = WindowFlags{};

	if ((sdl_window_flags & SDL_WINDOW_ALLOW_HIGHDPI) != 0)
	{
		flags |= WindowFlags::high_dpi;
	}

	if ((sdl_window_flags & SDL_WINDOW_SHOWN) != 0)
	{
		flags |= WindowFlags::shown;
	}

	if ((sdl_window_flags & SDL_WINDOW_HIDDEN) != 0)
	{
		flags |= WindowFlags::hidden;
	}

	if ((sdl_window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0)
	{
		flags |= WindowFlags::fullscreen_desktop;
	}

	if ((sdl_window_flags & SDL_WINDOW_BORDERLESS) != 0)
	{
		flags |= WindowFlags::borderless;
	}

	if ((sdl_window_flags & SDL_WINDOW_OPENGL) != 0)
	{
		flags |= WindowFlags::opengl;
	}

	return flags;
}

int sdl_get_gl_attr(SDL_GLattr gl_attrib)
try
{
	auto gl_value = 0;
	sdl_ensure_result(SDL_GL_GetAttribute(gl_attrib, &gl_value));
	return gl_value;
}
BSTONE_FUNC_STATIC_THROW_NESTED

bool sdl_get_gl_attr_bool(SDL_GLattr gl_attrib)
try
{
	switch (sdl_get_gl_attr(gl_attrib))
	{
		case SDL_FALSE: return false;
		case SDL_TRUE: return true;
		default: BSTONE_STATIC_THROW("Invalid boolean value.");
	}
}
BSTONE_FUNC_STATIC_THROW_NESTED

int sdl_map_gl_profile(GlContextProfile context_profile)
try
{
	switch (context_profile)
	{
		case GlContextProfile::compatibility: return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
		case GlContextProfile::core: return SDL_GL_CONTEXT_PROFILE_CORE;
		case GlContextProfile::es: return SDL_GL_CONTEXT_PROFILE_ES;
		default: BSTONE_STATIC_THROW("Unknown context profile.");
	}
}
BSTONE_FUNC_STATIC_THROW_NESTED

GlContextProfile sdl_map_gl_profile(int sdl_context_profile)
try
{
	switch (sdl_context_profile)
	{
		case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY: return GlContextProfile::compatibility;
		case SDL_GL_CONTEXT_PROFILE_CORE: return GlContextProfile::core;
		case SDL_GL_CONTEXT_PROFILE_ES: return GlContextProfile::es;
		default: BSTONE_STATIC_THROW("Unknown SDL context profile.");
	}
}
BSTONE_FUNC_STATIC_THROW_NESTED

SdlMakeWindowResult sdl_make_window_internal(Logger& logger, const GlWindowInitParam& param)
try
{
	const auto sdl_x = sdl_make_window_position_coord(param.rect.offset.x, param.position_x_type);
	const auto sdl_y = sdl_make_window_position_coord(param.rect.offset.y, param.position_y_type);

	const auto sdl_flags = sdl_map_window_flags(param.flags);
	const auto is_opengl = (sdl_flags & SDL_WINDOW_OPENGL) != 0;
	auto result = SdlMakeWindowResult{};

	if (is_opengl)
	{
		if (param.gl_attribs == nullptr)
		{
			BSTONE_STATIC_THROW("Null context attributes.");
		}

		const auto& src_attribs = *param.gl_attribs;
		const auto sdl_profile = sdl_map_gl_profile(src_attribs.profile);
		const auto sdl_accelerated = src_attribs.is_accelerated ? SDL_TRUE : SDL_FALSE;

		SDL_GL_ResetAttributes();
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, sdl_accelerated));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, sdl_profile));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, src_attribs.major_version));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, src_attribs.minor_version));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, src_attribs.multisample_buffer_count));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, src_attribs.multisample_sample_count));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, src_attribs.red_bit_count));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, src_attribs.green_bit_count));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, src_attribs.blue_bit_count));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, src_attribs.alpha_bit_count));
		sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, src_attribs.depth_bit_count));
	}

	result.sdl_window = SdlWindowUPtr{sdl_ensure_result(SDL_CreateWindow(
		param.title,
		sdl_x,
		sdl_y,
		param.rect.extent.width,
		param.rect.extent.height,
		sdl_flags))};

	if (is_opengl)
	{
		result.sdl_gl_context.reset(sdl_ensure_result(SDL_GL_CreateContext(result.sdl_window.get())));

		auto& dst_attribs = result.gl_attribs;
		dst_attribs.is_accelerated = sdl_get_gl_attr_bool(SDL_GL_ACCELERATED_VISUAL);
		dst_attribs.profile = sdl_map_gl_profile(sdl_get_gl_attr(SDL_GL_CONTEXT_PROFILE_MASK));
		dst_attribs.major_version = sdl_get_gl_attr(SDL_GL_CONTEXT_MAJOR_VERSION);
		dst_attribs.minor_version = sdl_get_gl_attr(SDL_GL_CONTEXT_MINOR_VERSION);
		dst_attribs.multisample_buffer_count = sdl_get_gl_attr(SDL_GL_MULTISAMPLEBUFFERS);
		dst_attribs.multisample_sample_count = sdl_get_gl_attr(SDL_GL_MULTISAMPLESAMPLES);
		dst_attribs.red_bit_count = sdl_get_gl_attr(SDL_GL_RED_SIZE);
		dst_attribs.green_bit_count = sdl_get_gl_attr(SDL_GL_GREEN_SIZE);
		dst_attribs.blue_bit_count = sdl_get_gl_attr(SDL_GL_BLUE_SIZE);
		dst_attribs.alpha_bit_count = sdl_get_gl_attr(SDL_GL_ALPHA_SIZE);
		dst_attribs.depth_bit_count = sdl_get_gl_attr(SDL_GL_DEPTH_SIZE);

		SDL_GL_GetDrawableSize(
			result.sdl_window.get(),
			&result.gl_drawable_size.width,
			&result.gl_drawable_size.height);
	}

	SDL_GetWindowSize(result.sdl_window.get(), &result.size.width, &result.size.height);

	const auto sdl_effective_flags = SDL_GetWindowFlags(result.sdl_window.get());
	result.flags = sdl_map_window_flags(sdl_effective_flags);
	return result;
}
BSTONE_FUNC_STATIC_THROW_NESTED

void sdl_log_window_param_internal(Logger& logger, const GlWindowInitParam& gl_param)
{
}

void sdl_log_window_param(Logger& logger, const WindowInitParam& param)
{
	logger.log_information("<<< Make SDL window.");
	auto gl_param = GlWindowInitParam{};
	static_cast<WindowInitParam&>(gl_param) = param;
	gl_param.gl_attribs = nullptr;
	sdl_log_window_param_internal(logger, gl_param);
}

void sdl_log_window_param(Logger& logger, const GlWindowInitParam& gl_param)
{
	logger.log_information("<<< Make SDL window (OpenGL).");
	sdl_log_window_param_internal(logger, gl_param);
}

} // namespace

SdlMakeWindowResult sdl_make_window(Logger& logger, const WindowInitParam& param)
try
{
	sdl_log_window_param(logger, param);

	if ((param.flags & WindowFlags::opengl) != WindowFlags{})
	{
		BSTONE_STATIC_THROW("OpenGL not allowed for ordinary window.");
	}

	auto gl_param = GlWindowInitParam{};
	static_cast<WindowInitParam&>(gl_param) = param;
	gl_param.gl_attribs = nullptr;
	return sdl_make_window_internal(logger, gl_param);
}
BSTONE_FUNC_STATIC_THROW_NESTED

SdlMakeWindowResult sdl_make_window(Logger& logger, const GlWindowInitParam& gl_param)
{
	sdl_log_window_param(logger, gl_param);
	return sdl_make_window_internal(logger, gl_param);
}

} // namespace detail
} // namespace sys
} // namespace bstone
