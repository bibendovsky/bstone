/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <SDL_messagebox.h>
#include <SDL_version.h>
#include "bstone_exception.h"
#include "bstone_sys_message_box.h"
#include "bstone_sys_exception_sdl2.h"

#define BSTONE_SDL_2_0_12 SDL_VERSION_ATLEAST(2, 0, 12)

#if !BSTONE_SDL_2_0_12
#define SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT (0x80)
#endif

namespace bstone {
namespace sys {

void MessageBox::show_simple(
	const char* title,
	const char* message,
	MessageBoxType type)
try {
	auto sdl_flags = Uint32{SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT};

	switch (type)
	{
		case MessageBoxType::error: sdl_flags |= SDL_MESSAGEBOX_ERROR; break;
		case MessageBoxType::information: sdl_flags |= SDL_MESSAGEBOX_INFORMATION; break;
		case MessageBoxType::warning: sdl_flags |= SDL_MESSAGEBOX_WARNING; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown type.");
	}

	sdl2_ensure_result(SDL_ShowSimpleMessageBox(sdl_flags, title, message, nullptr));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int MessageBox::show(const MessageBoxInitParam& param)
try {
	auto sdl_message_box_flags = Uint32{SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT};

	switch (param.type)
	{
		case MessageBoxType::error: sdl_message_box_flags |= SDL_MESSAGEBOX_ERROR; break;
		case MessageBoxType::information: sdl_message_box_flags |= SDL_MESSAGEBOX_INFORMATION; break;
		case MessageBoxType::warning: sdl_message_box_flags |= SDL_MESSAGEBOX_WARNING; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown type.");
	}

	constexpr auto max_buttons = 8;

	if (param.buttons.get_size() > max_buttons)
	{
		BSTONE_THROW_STATIC_SOURCE("Too many buttons.");
	}

	SDL_MessageBoxButtonData sdl_buttons[max_buttons]{};
	auto sdl_button = sdl_buttons;

	for (const auto& button : param.buttons)
	{
		auto sdl_button_flags = Uint32{};

		if ((button.flags & MessageBoxButtonFlags::default_for_escape_key) != MessageBoxButtonFlags{})
		{
			sdl_button_flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
		}

		if ((button.flags & MessageBoxButtonFlags::default_for_return_key) != MessageBoxButtonFlags{})
		{
			sdl_button_flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
		}

		*sdl_button = SDL_MessageBoxButtonData{};
		sdl_button->flags = sdl_button_flags;
		sdl_button->buttonid = button.id;
		sdl_button->text = button.text;
		++sdl_button;
	}

	auto sdl_message_box = SDL_MessageBoxData{};
	sdl_message_box.flags = sdl_message_box_flags;
	sdl_message_box.title = param.title;
	sdl_message_box.message = param.message;
	sdl_message_box.numbuttons = static_cast<int>(param.buttons.get_size());
	sdl_message_box.buttons = sdl_buttons;

	auto sdl_button_id = 0;
	sdl2_ensure_result(SDL_ShowMessageBox(&sdl_message_box, &sdl_button_id));
	return sdl_button_id;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
