/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_mouse.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_mouse_mgr_sdl2.h"

namespace bstone {
namespace sys {

namespace {

class Sdl2MouseMgr final : public MouseMgr
{
public:
	Sdl2MouseMgr(Logger& logger);
	~Sdl2MouseMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;

private:
	void do_set_relative_mode(bool is_enable) override;

private:
	static MemoryResource& get_memory_resource();
};

// ==========================================================================

Sdl2MouseMgr::Sdl2MouseMgr(Logger& logger)
try
	:
	logger_{logger}
{
	logger_.log_information("Start up SDL mouse manager.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2MouseMgr::~Sdl2MouseMgr()
{
	logger_.log_information("Shut down SDL mouse manager.");
}

void* Sdl2MouseMgr::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2MouseMgr::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

void Sdl2MouseMgr::do_set_relative_mode(bool is_enable)
{
	sdl2_ensure_result(SDL_SetRelativeMouseMode(is_enable ? SDL_TRUE : SDL_FALSE));
}

MemoryResource& Sdl2MouseMgr::get_memory_resource()
{
	static SinglePoolResource<Sdl2MouseMgr> memory_pool{};

	return memory_pool;
}

} // namespace

// ==========================================================================

MouseMgrUPtr make_sdl2_mouse_mgr(Logger& logger)
{
	return std::make_unique<Sdl2MouseMgr>(logger);
}

} // namespace sys
} // namespace bstone
