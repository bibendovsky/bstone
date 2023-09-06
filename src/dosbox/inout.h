/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// A stub for DOSBox OPL2 emulator.

#if !defined(BSTONE_DOSBOX_INOUT_INCLUDED)
#define BSTONE_DOSBOX_INOUT_INCLUDED

#include "dosbox.h"

using IO_ReadHandler = Bitu (*)(Bitu port, Bitu iolen);
using IO_WriteHandler = void (*)(Bitu port, Bitu val, Bitu iolen);

struct IO_ReadHandleObject {};
struct IO_WriteHandleObject {};

#endif // BSTONE_DOSBOX_INOUT_INCLUDED
