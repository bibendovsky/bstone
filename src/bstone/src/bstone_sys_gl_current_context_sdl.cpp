/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL current context (SDL).

#include "bstone_exception.h"
#include "bstone_sys_gl_current_context_sdl.h"
#include "bstone_sys_gl_symbol_resolver_sdl.h"
#include "bstone_sdl.h"
#include "SDL3/SDL_video.h"

namespace bstone::sys {

namespace {

class GlCurrentContextSdl final : public GlCurrentContext
{
public:
	GlCurrentContextSdl() = default;
	GlCurrentContextSdl(const GlCurrentContextSdl&) = delete;
	GlCurrentContextSdl& operator=(const GlCurrentContextSdl&) = delete;
	~GlCurrentContextSdl() override = default;

	bool has_extension(const char* extension_name) const override;
	SwapIntervalType get_swap_interval() const override;
	void set_swap_interval(SwapIntervalType swap_interval_type) override;
	const GlSymbolResolver& get_symbol_resolver() const override;

private:
	GlSymbolResolverSdl gl_symbol_resolver_{};

	static int map(SwapIntervalType swap_interval_type);
};

// --------------------------------------

bool GlCurrentContextSdl::has_extension(const char* extension_name) const
{
	return SDL_GL_ExtensionSupported(extension_name);
}

SwapIntervalType GlCurrentContextSdl::get_swap_interval() const
{
	int sdl_swap_interval;
	if (!SDL_GL_GetSwapInterval(&sdl_swap_interval))
	{
		sdl::fail("SDL_GL_GetSwapInterval");
	}
	switch (sdl_swap_interval)
	{
		case -1: return SwapIntervalType::adaptive;
		case 1: return SwapIntervalType::standard;
		default: return SwapIntervalType::none;
	}
}

void GlCurrentContextSdl::set_swap_interval(SwapIntervalType swap_interval_type)
{
	const int sdl_swap_interval = map(swap_interval_type);
	if (!SDL_GL_SetSwapInterval(sdl_swap_interval))
	{
		sdl::fail("SDL_GL_SetSwapInterval");
	}
}

const GlSymbolResolver& GlCurrentContextSdl::get_symbol_resolver() const
{
	return gl_symbol_resolver_;
}

int GlCurrentContextSdl::map(SwapIntervalType swap_interval_type)
{
	switch (swap_interval_type)
	{
		case SwapIntervalType::none: return 0;
		case SwapIntervalType::standard: return 1;
		case SwapIntervalType::adaptive: return -1;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown swap interval type.");
	}
}

} // namespace

// ======================================

GlCurrentContextUPtr make_gl_current_context_sdl([[maybe_unused]] Logger& logger)
{
	return std::make_unique<GlCurrentContextSdl>();
}

} // namespace bstone::sys
