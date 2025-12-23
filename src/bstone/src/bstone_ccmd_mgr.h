/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CCMD_MGR_INCLUDED
#define BSTONE_CCMD_MGR_INCLUDED

#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include "bstone_ccmd.h"

namespace bstone {

using CCmdMgrCCmds = std::span<CCmd*>;

// ==========================================================================

class CCmdMgr
{
public:
	CCmdMgr();
	virtual ~CCmdMgr();

	virtual CCmd* find(std::string_view name) const noexcept = 0;
	virtual CCmdMgrCCmds get_all() noexcept = 0;

	virtual void add(CCmd& ccmd) = 0;
};

// ==========================================================================

using CCmdMgrUPtr = std::unique_ptr<CCmdMgr>;

CCmdMgrUPtr make_ccmd_mgr(std::intptr_t max_ccmds);

} // namespace bstone

#endif // !BSTONE_CCMD_MGR_INCLUDED
