/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_DETAIL_SDL_INCLUDED
#define BSTONE_SYS_DETAIL_SDL_INCLUDED

#include <charconv>
#include <iterator>
#include <string>
#include "SDL3/SDL_render.h"
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
	constexpr int max_chars = 32;
	char char_buffer[max_chars];
	char* chars = char_buffer;
	switch (base)
	{
		case 8:
			chars[0] = '0';
			++chars;
			break;
		case 16:
			chars[0] = '0';
			chars[1] = 'x';
			chars += 2;
			break;
	}
	if (const auto [ptr, ec] = std::to_chars(chars, std::end(char_buffer), value, base);
		ec == std::errc{})
	{
		message.append(char_buffer, static_cast<std::size_t>(ptr - char_buffer));
	}
	else
	{
		message.append("???");
	}
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

#endif // BSTONE_SYS_DETAIL_SDL_INCLUDED
