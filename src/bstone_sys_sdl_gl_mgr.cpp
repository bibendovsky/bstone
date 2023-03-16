/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_video.h"
#include "bstone_memory_pool_1x.h"
#include "bstone_sys_sdl_exception.h"
#include "bstone_sys_sdl_gl_mgr.h"

namespace bstone {
namespace sys {

namespace {

class SdlGlMgr final : public GlMgr
{
public:
	SdlGlMgr(Logger& logger);
	SdlGlMgr(const SdlGlMgr&) = delete;
	SdlGlMgr& operator=(const SdlGlMgr&) = delete;
	~SdlGlMgr() override;

	static void* operator new(std::size_t count);
	static void operator delete(void* ptr) noexcept;

private:
	Logger& logger_;

private:
	void do_load_library(const char* path) override;
	void do_unload_library() noexcept override;

	bool do_has_extension(const char* extension_name) override;
	void* do_get_symbol(const char* symbol_name) override;

	int do_get_swap_interval() noexcept override;
	void do_set_swap_interval(int swap_interval) override;
};

// ==========================================================================

using SdlGlMgrPool = MemoryPool1XT<SdlGlMgr>;

SdlGlMgrPool sdl_gl_mgr_pool{};

// ==========================================================================

SdlGlMgr::SdlGlMgr(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Start up SDL OpenGL manager.");
}

SdlGlMgr::~SdlGlMgr()
{
	logger_.log_information("Shut down SDL OpenGL manager.");
}

void* SdlGlMgr::operator new(std::size_t count)
try
{
	return sdl_gl_mgr_pool.allocate(count);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void SdlGlMgr::operator delete(void* ptr) noexcept
{
	sdl_gl_mgr_pool.deallocate(ptr);
}

void SdlGlMgr::do_load_library(const char* path)
{
	sdl_ensure_result(SDL_GL_LoadLibrary(path));
}

void SdlGlMgr::do_unload_library() noexcept
{
	SDL_GL_UnloadLibrary();
}

bool SdlGlMgr::do_has_extension(const char* extension_name)
{
	return SDL_GL_ExtensionSupported(extension_name) == SDL_TRUE;
}

void* SdlGlMgr::do_get_symbol(const char* symbol_name)
{
	return SDL_GL_GetProcAddress(symbol_name);
}

int SdlGlMgr::do_get_swap_interval() noexcept
{
	return SDL_GL_GetSwapInterval();
}

void SdlGlMgr::do_set_swap_interval(int swap_interval)
{
	sdl_ensure_result(SDL_GL_SetSwapInterval(swap_interval));
}

} // namespace

// ==========================================================================

GlMgrUPtr make_sdl_gl_mgr(Logger& logger)
{
	return std::make_unique<SdlGlMgr>(logger);
}

} // namespace sys
} // namespace bstone
