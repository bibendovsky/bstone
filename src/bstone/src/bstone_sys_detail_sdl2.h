/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_DETAIL_SDL2_INCLUDED
#define BSTONE_SYS_DETAIL_SDL2_INCLUDED

#include <iterator>
#include <string>
#include "SDL_render.h"
#include "bstone_char_conv.h"
#include "bstone_sys_gl_context_attributes.h"
#include "bstone_sys_pixel_format.h"

namespace bstone {
namespace sys {
namespace detail {

void sdl2_log_eol(std::string& message);

void sdl2_log_bool(bool value, std::string& message);

template<typename T>
void sdl2_log_xint(T value, int base, std::string& message)
{
	constexpr auto max_chars = 32;
	char char_buffer[max_chars];
	auto chars = char_buffer;

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

	const auto char_count = to_chars(value, chars, std::end(char_buffer), base) - chars;
	message.append(chars, static_cast<std::size_t>(char_count));
}

template<typename T>
void sdl2_log_xint(T value, std::string& message)
{
	sdl2_log_xint(value, 10, message);
}

template<typename T>
void sdl2_log_xint_hex(T value, std::string& message)
{
	sdl2_log_xint(value, 16, message);
}

void sdl2_log_gl_attributes(const GlContextAttributes& gl_attribs, std::string& message);

} // namespace detail
} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_DETAIL_SDL2_INCLUDED
