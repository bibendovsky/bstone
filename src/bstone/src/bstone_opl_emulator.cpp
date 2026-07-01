/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OPL emulator interface

#include "bstone_opl_emulator.h"
#include "bstone_dbopl_emulator.h"
#include "bstone_nuked_opl3.h"

namespace bstone {

OplEmulatorUPtr make_opl_emulator(OplEmulatorType opl_emulator_type)
{
	switch (opl_emulator_type)
	{
		case OplEmulatorType::dbopl: return make_dbopl_emulator();
		case OplEmulatorType::nuked_opl3: return make_nuked_opl3();
		default: return nullptr;
	}
}

} // namespace bstone
