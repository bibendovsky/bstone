/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Window rounded corner manager (null).

#ifndef _WIN32

#include "bstone_sys_window_rounded_corner_mgr.h"

#include <cstddef>

#include "bstone_single_pool_resource.h"

namespace bstone {
namespace sys {

namespace {

class NullWindowRoundedCornerMgr final : public WindowRoundedCornerMgr
{
public:
	NullWindowRoundedCornerMgr();
	~NullWindowRoundedCornerMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr) noexcept;

private:
	using Pool = SinglePoolResource<NullWindowRoundedCornerMgr>;

private:
	void do_set_round_corner_type(Window&, WindowRoundedCornerType) override {}

private:
	static Pool pool_;
};

// --------------------------------------------------------------------------

NullWindowRoundedCornerMgr::Pool NullWindowRoundedCornerMgr::pool_{};

// --------------------------------------------------------------------------

NullWindowRoundedCornerMgr::NullWindowRoundedCornerMgr() = default;

NullWindowRoundedCornerMgr::~NullWindowRoundedCornerMgr() = default;

void* NullWindowRoundedCornerMgr::operator new(std::size_t size)
{
	return pool_.allocate(size);
}

void NullWindowRoundedCornerMgr::operator delete(void* ptr) noexcept
{
	pool_.deallocate(ptr);
}

} // namespace

// ==========================================================================

WindowRoundedCornerMgrUPtr make_window_rounded_corner_mgr()
{
	return std::make_unique<NullWindowRoundedCornerMgr>();
}

} // namespace sys
} // namespace bstone

#endif // _WIN32
