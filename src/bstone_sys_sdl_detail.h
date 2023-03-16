/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_SDL_DETAIL_INCLUDED)
#define BSTONE_SYS_SDL_DETAIL_INCLUDED

#include <string>
#include "SDL_render.h"
#include "bstone_char_conv.h"
#include "bstone_sys_gl_context_attributes.h"
#include "bstone_sys_pixel_format.h"

namespace bstone {
namespace sys {
namespace detail {

void sdl_log_eol(std::string& message);

void sdl_log_bool(bool value, std::string& message);

template<typename T>
void sdl_log_xint(T value, int base, std::string& message)
{
	constexpr auto max_chars = 32;
	char chars[max_chars];
	const auto char_count = char_conv::to_chars(value, make_span(chars), base, char_conv::ToCharsFormat::prefix);
	message.append(chars, static_cast<std::size_t>(char_count));
}

template<typename T>
void sdl_log_xint(T value, std::string& message)
{
	sdl_log_xint(value, 10, message);
}

template<typename T>
void sdl_log_xint_hex(T value, std::string& message)
{
	sdl_log_xint(value, 16, message);
}

void sdl_log_gl_attributes(const GlContextAttributes& gl_attribs, std::string& message);

} // namespace detail
} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL_DETAIL_INCLUDED
