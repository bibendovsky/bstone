/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// The launcher's icon. BStone's own artwork, not the game's.

#ifndef BSTONE_LAUNCHER_ICON_INCLUDED
#define BSTONE_LAUNCHER_ICON_INCLUDED

#include <span>

namespace bstone {

std::span<const unsigned char> get_launcher_icon_png();

} // namespace bstone

#endif // BSTONE_LAUNCHER_ICON_INCLUDED
