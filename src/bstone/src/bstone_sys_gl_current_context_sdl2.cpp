/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL current context (SDL).

#include <SDL_video.h>

#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_gl_current_context_sdl2.h"
#include "bstone_sys_gl_symbol_resolver_sdl2.h"

namespace bstone {
namespace sys {

class Sdl2GlCurrentContext final : public GlCurrentContext
{
public:
	Sdl2GlCurrentContext(Logger& logger);
	~Sdl2GlCurrentContext() override;

	static void* operator new(std::size_t size);
	static void operator delete(void* ptr) noexcept;

private:
	Logger& logger_;
	Sdl2GlSymbolResolver gl_symbol_resolver_{};

	static SinglePoolResource<Sdl2GlCurrentContext> memoryResource_;

private:
	bool do_has_extension(const char* extension_name) const noexcept override;

	SwapIntervalType do_get_swap_interval() const noexcept override;
	void do_set_swap_interval(SwapIntervalType swap_interval_type) override;

	const GlSymbolResolver& do_get_symbol_resolver() const noexcept override;

private:
	static int map(SwapIntervalType swap_interval_type);
};

// --------------------------------------------------------------------------

SinglePoolResource<Sdl2GlCurrentContext> Sdl2GlCurrentContext::memoryResource_{};

Sdl2GlCurrentContext::Sdl2GlCurrentContext(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Start up OpenGL current context.");
}

Sdl2GlCurrentContext::~Sdl2GlCurrentContext()
{
	logger_.log_information("Shut down OpenGL current context.");
}

void* Sdl2GlCurrentContext::operator new(std::size_t size)
{
	return memoryResource_.allocate(static_cast<std::intptr_t>(size));
}

void Sdl2GlCurrentContext::operator delete(void* ptr) noexcept
{
	memoryResource_.deallocate(ptr);
}

bool Sdl2GlCurrentContext::do_has_extension(const char* extension_name) const noexcept
{
	return SDL_GL_ExtensionSupported(extension_name) == SDL_TRUE;
}

SwapIntervalType Sdl2GlCurrentContext::do_get_swap_interval() const noexcept
{
	const auto sdl_swap_interval = SDL_GL_GetSwapInterval();

	switch (sdl_swap_interval)
	{
		case -1: return SwapIntervalType::adaptive;
		case 1: return SwapIntervalType::standard;
		default: return SwapIntervalType::none;
	}
}

void Sdl2GlCurrentContext::do_set_swap_interval(SwapIntervalType swap_interval_type)
{
	const auto sdl_swap_interval = map(swap_interval_type);
	sdl2_ensure_result(SDL_GL_SetSwapInterval(sdl_swap_interval));
}

const GlSymbolResolver& Sdl2GlCurrentContext::do_get_symbol_resolver() const noexcept
{
	return gl_symbol_resolver_;
}

int Sdl2GlCurrentContext::map(SwapIntervalType swap_interval_type)
{
	switch (swap_interval_type)
	{
		case SwapIntervalType::none: return 0;
		case SwapIntervalType::standard: return 1;
		case SwapIntervalType::adaptive: return -1;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown 3D renderer swap interval type.");
	}
}

GlCurrentContextUPtr make_sdl2_gl_current_context(Logger& logger)
{
	return std::make_unique<Sdl2GlCurrentContext>(logger);
}

} // namespace sys
} // namespace bstone
