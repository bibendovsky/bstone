/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// DOSBox DBOPL wrapper.
//

#ifndef BSTONE_DOSBOX_DBOPL_INCLUDED
#define BSTONE_DOSBOX_DBOPL_INCLUDED

#include "bstone_opl3.h"

namespace bstone
{

Opl3UPtr make_dbopl_opl3();

} // bstone

#endif // !BSTONE_DOSBOX_DBOPL_INCLUDED
