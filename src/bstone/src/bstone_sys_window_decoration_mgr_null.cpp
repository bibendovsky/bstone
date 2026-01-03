/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window decoration manager (NULL)

#ifndef _WIN32

#include "bstone_sys_window_decoration_mgr.h"

namespace bstone::sys {

namespace {

class NullWindowDecorationMgr final : public WindowDecorationMgr
{
public:
	NullWindowDecorationMgr() = default;
	~NullWindowDecorationMgr() override = default;

private:
	void do_set_round_corner_type(Window& window, WindowRoundedCornerType round_corner_type) override;
};

// --------------------------------------

void NullWindowDecorationMgr::do_set_round_corner_type(
	[[maybe_unused]] Window& window,
	[[maybe_unused]] WindowRoundedCornerType round_corner_type)
{}

} // namespace

// ======================================

WindowDecorationMgrUPtr make_window_decoration_mgr()
{
	return std::make_unique<NullWindowDecorationMgr>();
}

} // namespace bstone::sys

#endif // _WIN32
