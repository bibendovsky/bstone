/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// OPL3 emulator interface.
//

#include "bstone_opl3.h"
#include "bstone_dosbox_dbopl.h"
#include "bstone_nuked_opl3.h"

namespace bstone
{

Opl3::Opl3() noexcept = default;

Opl3::~Opl3() = default;

// ==========================================================================

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
