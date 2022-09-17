/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// OPL3 emulator interface.
//

#include "bstone_opl3.h"
#include "bstone_dosbox_dbopl.h"
#include "bstone_nuked_opl3.h"

namespace bstone
{

Opl3UPtr make_opl3(Opl3Type opl3_type)
{
	switch (opl3_type)
	{
		case Opl3Type::dbopl:
			return make_dbopl_opl3();

		case Opl3Type::nuked:
			return make_nuked_opl3();

		default:
			return nullptr;
	}
}

} // bstone
