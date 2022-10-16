/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_cvar_mgr.h"
#include "bstone_page_mgr.h"

namespace bstone {
namespace globals {

constexpr auto max_cvars = 1024;

extern CVarMgrUPtr cvar_mgr;
extern PageMgrUPtr page_mgr;

} // globals
} // bstone
