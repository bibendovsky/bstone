/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_single_pool_memory_resource.h"
#include "bstone_sys_window_sdl2.h"
#include "bstone_sys_window_mgr_sdl2.h"

namespace bstone {
namespace sys {

namespace {

class Sdl2WindowMgr final : public WindowMgr
{
public:
	Sdl2WindowMgr(Logger& logger);
	~Sdl2WindowMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;

private:
	WindowUPtr do_make_window(const WindowInitParam& param) override;

private:
	static MemoryResource& get_memory_resource();
};

// ==========================================================================

Sdl2WindowMgr::Sdl2WindowMgr(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Start up SDL window manager.");
}

Sdl2WindowMgr::~Sdl2WindowMgr()
{
	logger_.log_information("Shut down SDL window manager.");
}

void* Sdl2WindowMgr::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2WindowMgr::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

WindowUPtr Sdl2WindowMgr::do_make_window(const WindowInitParam& param)
{
	return make_sdl2_window(logger_, param);
}

MemoryResource& Sdl2WindowMgr::get_memory_resource()
{
	static SinglePoolMemoryResource<Sdl2WindowMgr> memory_pool{};

	return memory_pool;
}

} // namespace

// ==========================================================================

WindowMgrUPtr make_sdl2_window_mgr(Logger& logger)
{
	return std::make_unique<Sdl2WindowMgr>(logger);
}

} // namespace sys
} // namespace bstone
