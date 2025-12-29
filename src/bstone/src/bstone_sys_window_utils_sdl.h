/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_SDL_WINDOW_UTILS_INCLUDED)
#define BSTONE_SYS_SDL_WINDOW_UTILS_INCLUDED

#include "bstone_sdl_types.h"
#include "bstone_sys_gl_context_attributes.h"
#include "bstone_sys_gl_window.h"
#include "bstone_sys_logger.h"

namespace bstone {
namespace sys {
namespace detail {

struct SdlMakeWindowResult
{
	WindowFlags flags{};
	R2Extent size{};
	R2Extent gl_drawable_size{};
	GlContextAttributes gl_attribs{};
	SdlWindowUPtr sdl_window{};
	SdlGlContextUPtr sdl_gl_context{};
};

SdlMakeWindowResult sdl_make_window(Logger& logger, const WindowInitParam& param);
SdlMakeWindowResult sdl_make_window(Logger& logger, const GlWindowInitParam& gl_param);

} // namespace detail
} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL_WINDOW_UTILS_INCLUDED
