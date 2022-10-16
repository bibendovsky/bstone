/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CVAR_MGR_INCLUDED
#define BSTONE_CVAR_MGR_INCLUDED

#include <memory>
#include "bstone_cvar.h"
#include "bstone_int.h"
#include "bstone_span.h"
#include "bstone_string_view.h"

namespace bstone {

using CVarMgrCVars = Span<CVar*>;

// --------------------------------------------------------------------------

class CVarMgr
{
public:
	CVarMgr() = default;
	virtual ~CVarMgr() = default;

	virtual CVar* find(StringView name) const noexcept = 0;
	virtual CVarMgrCVars get_all() noexcept = 0;

	virtual void add(CVar& cvar) = 0;
};

// ==========================================================================

using CVarMgrUPtr = std::unique_ptr<CVarMgr>;

CVarMgrUPtr make_cvar_mgr(Int max_cvars);

} // namespace bstone

#endif // !BSTONE_CVAR_MGR_INCLUDED
