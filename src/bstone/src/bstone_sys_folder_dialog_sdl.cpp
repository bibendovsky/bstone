/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Folder dialog (SDL)

#include "bstone_sys_folder_dialog.h"

#include "bstone_sdl.h"
#include "SDL3/SDL_dialog.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_timer.h"

namespace bstone::sys {

namespace {

struct FolderDialogState
{
	bool is_done;
	std::string path;
};

void SDLCALL folder_dialog_callback(void* userdata, const char* const* file_list, int)
{
	auto& state = *static_cast<FolderDialogState*>(userdata);

	// A null list is an error, an empty one is a cancellation; neither yields a path.
	if (file_list != nullptr && file_list[0] != nullptr)
	{
		state.path = file_list[0];
	}

	state.is_done = true;
}

} // namespace

std::string FolderDialog::show(const char* title, const char* default_path)
{
	static_cast<void>(title);

	// The dialog needs the video subsystem, which is up by the time anything
	// asks for game files, but a headless run has no dialog to show.
	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		return std::string{};
	}

	auto state = FolderDialogState{};
	SDL_ShowOpenFolderDialog(folder_dialog_callback, &state, nullptr, default_path, false);

	// The dialog is asynchronous everywhere; it reports back through the event
	// loop, so pump it until the callback has run.
	while (!state.is_done)
	{
		SDL_PumpEvents();
		SDL_Delay(10);
	}

	return state.path;
}

} // namespace bstone::sys
