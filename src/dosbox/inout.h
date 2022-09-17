/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// A stub for DOSBox OPL2 emulator.
//


#ifndef BSTONE_DOSBOX_INOUT_INCLUDED
#define BSTONE_DOSBOX_INOUT_INCLUDED


#include "dosbox.h"


typedef Bitu IO_ReadHandler(Bitu port, Bitu iolen);
typedef void IO_WriteHandler(Bitu port, Bitu val, Bitu iolen);


struct IO_ReadHandleObject
{
}; // IO_ReadHandleObject


struct IO_WriteHandleObject
{
}; // IO_WriteHandleObject


#endif // BSTONE_DOSBOX_INOUT_INCLUDED
