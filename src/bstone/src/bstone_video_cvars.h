/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Video cvars.

#ifndef BSTONE_VIDEO_CVARS_INCLUDED
#define BSTONE_VIDEO_CVARS_INCLUDED

#include "bstone_cvar.h"
#include "bstone_string_view.h"

namespace bstone {

// ==========================================================================
// vid_refresh_rate

constexpr int vid_refresh_rate_cvar_default = 0;
constexpr int vid_refresh_rate_min_value = 0;
constexpr int vid_refresh_rate_max_value = 1000;

extern bstone::CVar vid_refresh_rate_cvar;

// ==========================================================================
// vid_window_mode

constexpr bstone::StringView vid_window_mode_cvar_windowed = "windowed";
constexpr bstone::StringView vid_window_mode_cvar_fullscreen = "fullscreen";
constexpr bstone::StringView vid_window_mode_cvar_fake_fullscreen = "fake_fullscreen";
constexpr bstone::StringView vid_window_mode_cvar_default = vid_window_mode_cvar_windowed;

extern bstone::CVar vid_window_mode_cvar;

} // namespace bstone

#endif // BSTONE_VIDEO_CVARS_INCLUDED
