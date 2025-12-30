/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Message box (SDL)

#include "bstone_exception.h"
#include "bstone_sys_message_box.h"
#include <format>
#include <string>
#include "SDL3/SDL_messagebox.h"

namespace bstone::sys {

void MessageBox::show_simple(const char* title, const char* message, MessageBoxType type)
{
	Uint32 sdl_flags = SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
	switch (type)
	{
		case MessageBoxType::error:
			sdl_flags |= SDL_MESSAGEBOX_ERROR;
			break;
		case MessageBoxType::information:
			sdl_flags |= SDL_MESSAGEBOX_INFORMATION;
			break;
		case MessageBoxType::warning:
			sdl_flags |= SDL_MESSAGEBOX_WARNING;
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown message box type.");
	}
	if (!SDL_ShowSimpleMessageBox(sdl_flags, title, message, nullptr))
	{
		const std::string error_message = std::format("[{}] {}", "SDL_ShowSimpleMessageBox", SDL_GetError());
		BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
	}
}

int MessageBox::show(const MessageBoxInitParam& param)
{
	Uint32 sdl_message_box_flags = SDL_MESSAGEBOX_BUTTONS_LEFT_TO_RIGHT;
	switch (param.type)
	{
		case MessageBoxType::error:
			sdl_message_box_flags |= SDL_MESSAGEBOX_ERROR;
			break;
		case MessageBoxType::information:
			sdl_message_box_flags |= SDL_MESSAGEBOX_INFORMATION;
			break;
		case MessageBoxType::warning:
			sdl_message_box_flags |= SDL_MESSAGEBOX_WARNING;
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown message box type.");
	}
	constexpr int max_buttons = 8;
	if (param.buttons.size() > max_buttons)
	{
		BSTONE_THROW_STATIC_SOURCE("Too many buttons in message box.");
	}
	SDL_MessageBoxButtonData sdl_buttons[max_buttons]{};
	SDL_MessageBoxButtonData* sdl_button = sdl_buttons;
	for (const auto& button : param.buttons)
	{
		Uint32 sdl_button_flags = 0;
		if ((button.flags & MessageBoxButtonFlags::default_for_escape_key) != MessageBoxButtonFlags{})
		{
			sdl_button_flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
		}
		if ((button.flags & MessageBoxButtonFlags::default_for_return_key) != MessageBoxButtonFlags{})
		{
			sdl_button_flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
		}
		*sdl_button++ = SDL_MessageBoxButtonData{
			.flags = sdl_button_flags,
			.buttonID = button.id,
			.text = button.text,
		};
	}
	SDL_MessageBoxData sdl_message_box{
		.flags = sdl_message_box_flags,
		.title = param.title,
		.message = param.message,
		.numbuttons = static_cast<int>(param.buttons.size()),
		.buttons = sdl_buttons,
	};
	int sdl_button_id = 0;
	if (!SDL_ShowMessageBox(&sdl_message_box, &sdl_button_id))
	{
		const std::string message = std::format("[{}] {}", "SDL_ShowMessageBox", SDL_GetError());
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}
	return sdl_button_id;
}

} // namespace bstone::sys
