/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_single_memory_pool.h"
#include "bstone_sys_sdl_window.h"
#include "bstone_sys_sdl_window_mgr.h"

namespace bstone {
namespace sys {

namespace {

class SdlWindowMgr final : public WindowMgr
{
public:
	SdlWindowMgr(Logger& logger);
	~SdlWindowMgr() override;

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

SdlWindowMgr::SdlWindowMgr(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Start up SDL window manager.");
}

SdlWindowMgr::~SdlWindowMgr()
{
	logger_.log_information("Shut down SDL window manager.");
}

void* SdlWindowMgr::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindowMgr::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

WindowUPtr SdlWindowMgr::do_make_window(const WindowInitParam& param)
{
	return make_sdl_window(logger_, param);
}

MemoryResource& SdlWindowMgr::get_memory_resource()
{
	static SingleMemoryPool<SdlWindowMgr> memory_pool{};

	return memory_pool;
}

} // namespace

// ==========================================================================

WindowMgrUPtr make_sdl_window_mgr(Logger& logger)
{
	return std::make_unique<SdlWindowMgr>(logger);
}

} // namespace sys
} // namespace bstone
