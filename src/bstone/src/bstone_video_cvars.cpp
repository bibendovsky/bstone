/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Video cvars.

#include "bstone_video_cvars.h"

namespace bstone {

// ==========================================================================
// vid_refresh_rate

namespace {

constexpr bstone::StringView vid_refresh_rate_cvar_name{"vid_refresh_rate"};

} // namespace

bstone::CVar vid_refresh_rate_cvar{
	bstone::CVarInt32Tag{},
	vid_refresh_rate_cvar_name,
	bstone::CVarFlags::archive,
	vid_refresh_rate_cvar_default,
	vid_refresh_rate_min_value,
	vid_refresh_rate_max_value};

// ==========================================================================
// vid_window_mode

namespace {

constexpr bstone::StringView vid_window_mode_cvar_name{"vid_window_mode"};

constexpr bstone::StringView vid_window_mode_cvar_values[] =
{
	vid_window_mode_cvar_windowed,
	vid_window_mode_cvar_fullscreen,
	vid_window_mode_cvar_fake_fullscreen,
};

} // namespace

bstone::CVar vid_window_mode_cvar{
	bstone::CVarStringTag{},
	vid_window_mode_cvar_name,
	bstone::CVarFlags::archive,
	vid_window_mode_cvar_default,
	bstone::make_span(vid_window_mode_cvar_values)};

} // namespace bstone
