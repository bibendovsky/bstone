/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window rounded corner manager (NULL)

#ifndef _WIN32

#include "bstone_sys_window_rounded_corner_mgr.h"

namespace bstone::sys {

namespace {

class NullWindowRoundedCornerMgr final : public WindowRoundedCornerMgr
{
public:
	NullWindowRoundedCornerMgr() = default;
	~NullWindowRoundedCornerMgr() override = default;

private:
	void do_set_round_corner_type(Window& window, WindowRoundedCornerType round_corner_type) override;
};

// --------------------------------------

void NullWindowRoundedCornerMgr::do_set_round_corner_type(
	[[maybe_unused]] Window& window,
	[[maybe_unused]] WindowRoundedCornerType round_corner_type)
{}

} // namespace

// ======================================

WindowRoundedCornerMgrUPtr make_window_rounded_corner_mgr()
{
	return std::make_unique<NullWindowRoundedCornerMgr>();
}

} // namespace bstone::sys

#endif // _WIN32
