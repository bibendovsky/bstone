/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_mouse.h"
#include "bstone_exception.h"
#include "bstone_single_memory_pool.h"
#include "bstone_sys_sdl_exception.h"
#include "bstone_sys_sdl_mouse_mgr.h"

namespace bstone {
namespace sys {

namespace {

class SdlMouseMgr final : public MouseMgr
{
public:
	SdlMouseMgr(Logger& logger);
	~SdlMouseMgr() override;

	static void* operator new(std::size_t size);
	static void operator delete(void* ptr);

private:
	Logger& logger_;

private:
	void do_set_relative_mode(bool is_enable) override;
};

// ==========================================================================

using SdlMouseMgrPool = SingleMemoryPool<SdlMouseMgr>;
SdlMouseMgrPool sdl_mouse_mgr_pool{};

// ==========================================================================

SdlMouseMgr::SdlMouseMgr(Logger& logger)
BSTONE_BEGIN_CTOR_TRY
	:
	logger_{logger}
{
	logger_.log_information("Start up SDL mouse manager.");
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SdlMouseMgr::~SdlMouseMgr()
{
	logger_.log_information("Shut down SDL mouse manager.");
}

void* SdlMouseMgr::operator new(std::size_t size)
BSTONE_BEGIN_FUNC_TRY
	return sdl_mouse_mgr_pool.allocate(size);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlMouseMgr::operator delete(void* ptr)
BSTONE_BEGIN_FUNC_TRY
	sdl_mouse_mgr_pool.deallocate(ptr);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlMouseMgr::do_set_relative_mode(bool is_enable)
{
	sdl_ensure_result(SDL_SetRelativeMouseMode(is_enable ? SDL_TRUE : SDL_FALSE));
}

} // namespace

// ==========================================================================

MouseMgrUPtr make_sdl_mouse_mgr(Logger& logger)
{
	return std::make_unique<SdlMouseMgr>(logger);
}

} // namespace sys
} // namespace bstone
