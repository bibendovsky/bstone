/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// SDL utility

#include "bstone_sdl.h"
#include "bstone_exception.h"
#include <string>
#include "SDL3/SDL_error.h"

namespace bstone::sdl {

[[noreturn]] void fail(const char* caption)
{
	const std::size_t caption_length = std::string::traits_type::length(caption);
	const char* const sdl_error_message = SDL_GetError();
	const std::size_t sdl_error_message_length = std::string::traits_type::length(sdl_error_message);
	std::string message{};
	message.reserve(caption_length + sdl_error_message_length + 3);
	message += '[';
	message += caption;
	message += "] ";
	message += sdl_error_message;
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

} // namespace bstone::sdl
