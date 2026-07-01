/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// DOSBox DBOPL wrapper

#ifndef BSTONE_DOSBOX_DBOPL_INCLUDED
#define BSTONE_DOSBOX_DBOPL_INCLUDED

#include "bstone_opl_emulator.h"

namespace bstone {

OplEmulatorUPtr make_dbopl_opl3();

} // namespace bstone

#endif // BSTONE_DOSBOX_DBOPL_INCLUDED
