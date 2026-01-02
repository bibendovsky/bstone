/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL context (SDL)

#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_sys_gl_context_sdl.h"
#include <format>
#include <string>
#include <type_traits>

namespace bstone::sys {

namespace {

class GlContextSdl final : public GlContext
{
public:
	GlContextSdl(SDL_Window& sdl_window);
	GlContextSdl(const GlContextSdl&) = delete;
	GlContextSdl& operator=(const GlContextSdl&) = delete;
	~GlContextSdl() override;

private:
	SDL_GLContext sdl_gl_context_;
	GlContextAttributes gl_context_attributes_;

	const GlContextAttributes& do_get_attributes() const override;

	static const char* get_gl_attribute_name(SDL_GLAttr sdl_gl_attr);
	static GlContextProfile map_profile(SDL_GLProfile sdl_context_profile);
	static int get_gl_attribute(SDL_GLAttr sdl_gl_attr);
	static bool get_attrib_bool(SDL_GLAttr sdl_gl_attr);
};

// --------------------------------------

GlContextSdl::GlContextSdl(SDL_Window& sdl_window)
{
	SDL_GLContext sdl_gl_context = SDL_GL_CreateContext(&sdl_window);
	const auto scope_exit = make_scope_exit(
		[&sdl_gl_context]()
		{
			if (sdl_gl_context != nullptr)
			{
				SDL_GL_DestroyContext(sdl_gl_context);
			}
		});
	gl_context_attributes_.is_accelerated = get_attrib_bool(SDL_GL_ACCELERATED_VISUAL);
	gl_context_attributes_.profile = map_profile(get_gl_attribute(SDL_GL_CONTEXT_PROFILE_MASK));
	gl_context_attributes_.major_version = get_gl_attribute(SDL_GL_CONTEXT_MAJOR_VERSION);
	gl_context_attributes_.minor_version = get_gl_attribute(SDL_GL_CONTEXT_MINOR_VERSION);
	gl_context_attributes_.multisample_buffer_count = get_gl_attribute(SDL_GL_MULTISAMPLEBUFFERS);
	gl_context_attributes_.multisample_sample_count = get_gl_attribute(SDL_GL_MULTISAMPLESAMPLES);
	gl_context_attributes_.red_bit_count = get_gl_attribute(SDL_GL_RED_SIZE);
	gl_context_attributes_.green_bit_count = get_gl_attribute(SDL_GL_GREEN_SIZE);
	gl_context_attributes_.blue_bit_count = get_gl_attribute(SDL_GL_BLUE_SIZE);
	gl_context_attributes_.alpha_bit_count = get_gl_attribute(SDL_GL_ALPHA_SIZE);
	gl_context_attributes_.depth_bit_count = get_gl_attribute(SDL_GL_DEPTH_SIZE);
	sdl_gl_context_ = sdl_gl_context;
	sdl_gl_context = nullptr;
}

GlContextSdl::~GlContextSdl()
{
	SDL_GL_DestroyContext(sdl_gl_context_);
}

const GlContextAttributes& GlContextSdl::do_get_attributes() const
{
	return gl_context_attributes_;
}

const char* GlContextSdl::get_gl_attribute_name(SDL_GLAttr sdl_gl_attr)
{
#define BSTONE_MACRO(x) case x: return #x
	switch (sdl_gl_attr)
	{
		BSTONE_MACRO(SDL_GL_RED_SIZE);
		BSTONE_MACRO(SDL_GL_GREEN_SIZE);
		BSTONE_MACRO(SDL_GL_BLUE_SIZE);
		BSTONE_MACRO(SDL_GL_ALPHA_SIZE);
		BSTONE_MACRO(SDL_GL_BUFFER_SIZE);
		BSTONE_MACRO(SDL_GL_DOUBLEBUFFER);
		BSTONE_MACRO(SDL_GL_DEPTH_SIZE);
		BSTONE_MACRO(SDL_GL_STENCIL_SIZE);
		BSTONE_MACRO(SDL_GL_ACCUM_RED_SIZE);
		BSTONE_MACRO(SDL_GL_ACCUM_GREEN_SIZE);
		BSTONE_MACRO(SDL_GL_ACCUM_BLUE_SIZE);
		BSTONE_MACRO(SDL_GL_ACCUM_ALPHA_SIZE);
		BSTONE_MACRO(SDL_GL_STEREO);
		BSTONE_MACRO(SDL_GL_MULTISAMPLEBUFFERS);
		BSTONE_MACRO(SDL_GL_MULTISAMPLESAMPLES);
		BSTONE_MACRO(SDL_GL_ACCELERATED_VISUAL);
		BSTONE_MACRO(SDL_GL_RETAINED_BACKING);
		BSTONE_MACRO(SDL_GL_CONTEXT_MAJOR_VERSION);
		BSTONE_MACRO(SDL_GL_CONTEXT_MINOR_VERSION);
		BSTONE_MACRO(SDL_GL_CONTEXT_FLAGS);
		BSTONE_MACRO(SDL_GL_CONTEXT_PROFILE_MASK);
		BSTONE_MACRO(SDL_GL_SHARE_WITH_CURRENT_CONTEXT);
		BSTONE_MACRO(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE);
		BSTONE_MACRO(SDL_GL_CONTEXT_RELEASE_BEHAVIOR);
		BSTONE_MACRO(SDL_GL_CONTEXT_RESET_NOTIFICATION);
		BSTONE_MACRO(SDL_GL_CONTEXT_NO_ERROR);
		BSTONE_MACRO(SDL_GL_FLOATBUFFERS);
		BSTONE_MACRO(SDL_GL_EGL_PLATFORM);
		default: return nullptr;
	}
#undef BSTONE_MACRO
}

GlContextProfile GlContextSdl::map_profile(SDL_GLProfile sdl_context_profile)
{
	switch (sdl_context_profile)
	{
		case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY: return GlContextProfile::compatibility;
		case SDL_GL_CONTEXT_PROFILE_CORE: return GlContextProfile::core;
		case SDL_GL_CONTEXT_PROFILE_ES: return GlContextProfile::es;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown SDL context profile.");
	}
}

int GlContextSdl::get_gl_attribute(SDL_GLAttr sdl_gl_attr)
{
	int value;
	if (!SDL_GL_GetAttribute(sdl_gl_attr, &value))
	{
		const char* const attribute_name = get_gl_attribute_name(sdl_gl_attr);
		const std::string message = std::format(
			"[{}] {} (attr={} {})",
			"SDL_GL_GetAttribute",
			SDL_GetError(),
			attribute_name != nullptr ? attribute_name : "???",
			static_cast<std::underlying_type_t<decltype(sdl_gl_attr)>>(sdl_gl_attr));
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}
	return value;
}

bool GlContextSdl::get_attrib_bool(SDL_GLAttr sdl_gl_attr)
{
	return get_gl_attribute(sdl_gl_attr) != 0;
}

} // namespace

// ======================================

GlContextUPtr make_gl_context_sdl([[maybe_unused]] Logger& logger, SDL_Window& sdl_window)
{
	return std::make_unique<GlContextSdl>(sdl_window);
}

} // namespace bstone::sys
