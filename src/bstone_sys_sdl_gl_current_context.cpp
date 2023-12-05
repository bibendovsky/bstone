/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL current context (SDL).

#include <SDL_video.h>

#include "bstone_exception.h"
#include "bstone_single_pool_memory_resource.h"
#include "bstone_sys_sdl_exception.h"
#include "bstone_sys_sdl_gl_current_context.h"
#include "bstone_sys_sdl_gl_symbol_resolver.h"

namespace bstone {
namespace sys {

class SdlGlCurrentContext : public GlCurrentContext
{
public:
	SdlGlCurrentContext(Logger& logger);
	~SdlGlCurrentContext() override;

	static void* operator new(std::size_t size);
	static void operator delete(void* ptr) noexcept;

private:
	Logger& logger_;
	SdlGlSymbolResolver gl_symbol_resolver_{};

	static SinglePoolMemoryResource<SdlGlCurrentContext> memoryResource_;

private:
	bool do_has_extension(const char* extension_name) const noexcept override;

	R3rSwapIntervalType do_get_swap_interval() const noexcept override;
	void do_set_swap_interval(R3rSwapIntervalType swap_interval_type) override;

	const GlSymbolResolver& do_get_symbol_resolver() const noexcept override;

private:
	static int map(R3rSwapIntervalType swap_interval_type);
};

// --------------------------------------------------------------------------

SinglePoolMemoryResource<SdlGlCurrentContext> SdlGlCurrentContext::memoryResource_{};

SdlGlCurrentContext::SdlGlCurrentContext(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Start up OpenGL current context.");
}

SdlGlCurrentContext::~SdlGlCurrentContext()
{
	logger_.log_information("Shut down OpenGL current context.");
}

void* SdlGlCurrentContext::operator new(std::size_t size)
{
	return memoryResource_.allocate(static_cast<std::intptr_t>(size));
}

void SdlGlCurrentContext::operator delete(void* ptr) noexcept
{
	memoryResource_.deallocate(ptr);
}

bool SdlGlCurrentContext::do_has_extension(const char* extension_name) const noexcept
{
	return SDL_GL_ExtensionSupported(extension_name) == SDL_TRUE;
}

R3rSwapIntervalType SdlGlCurrentContext::do_get_swap_interval() const noexcept
{
	const auto sdl_swap_interval = SDL_GL_GetSwapInterval();

	switch (sdl_swap_interval)
	{
		case -1: return R3rSwapIntervalType::adaptive;
		case 1: return R3rSwapIntervalType::standard;
		default: return R3rSwapIntervalType::none;
	}
}

void SdlGlCurrentContext::do_set_swap_interval(R3rSwapIntervalType swap_interval_type)
{
	const auto sdl_swap_interval = map(swap_interval_type);
	sdl_ensure_result(SDL_GL_SetSwapInterval(sdl_swap_interval));
}

const GlSymbolResolver& SdlGlCurrentContext::do_get_symbol_resolver() const noexcept
{
	return gl_symbol_resolver_;
}

int SdlGlCurrentContext::map(R3rSwapIntervalType swap_interval_type)
{
	switch (swap_interval_type)
	{
		case R3rSwapIntervalType::none: return 0;
		case R3rSwapIntervalType::standard: return 1;
		case R3rSwapIntervalType::adaptive: return -1;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown 3D renderer swap interval type.");
	}
}

GlCurrentContextUPtr make_gl_current_context(Logger& logger)
{
	return std::make_unique<SdlGlCurrentContext>(logger);
}

} // namespace sys
} // namespace bstone
