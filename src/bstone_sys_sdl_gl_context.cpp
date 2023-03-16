/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <string>
#include <type_traits>
#include "bstone_exception.h"
#include "bstone_memory_pool_1x.h"
#include "bstone_sys_sdl_detail.h"
#include "bstone_sys_sdl_exception.h"
#include "bstone_sys_sdl_gl_context.h"

namespace bstone {
namespace sys {

namespace {

struct SdlGlContextDeleter
{
	void operator()(SDL_GLContext sdl_gl_context) const noexcept
	{
		SDL_GL_DeleteContext(sdl_gl_context);
	}
};

using SdlGlContextUPtr = std::unique_ptr<std::remove_pointer_t<SDL_GLContext>, SdlGlContextDeleter>;

// ==========================================================================

class SdlGlContext final : public GlContext
{
public:
	SdlGlContext(Logger& logger, SDL_Window& sdl_window);
	SdlGlContext(const SdlGlContext&) = delete;
	SdlGlContext& operator=(const SdlGlContext&) = delete;
	~SdlGlContext() override;

	static void* operator new(std::size_t count);
	static void operator delete(void* ptr) noexcept;

private:
	Logger& logger_;
	SdlGlContextUPtr sdl_context_{};
	GlContextAttributes attributes_{};

private:
	const GlContextAttributes& do_get_attributes() const noexcept override;

private:
	static GlContextProfile map_profile(int sdl_context_profile);
	static int get_attrib(SDL_GLattr gl_attrib);
	static bool get_attrib_bool(SDL_GLattr gl_attrib);
};

// ==========================================================================

using SdlGlContextPool = MemoryPool1XT<SdlGlContext>;

SdlGlContextPool sdl_gl_context_pool{};

// ==========================================================================

SdlGlContext::SdlGlContext(Logger& logger, SDL_Window& sdl_window)
try
	:
	logger_{logger}
{
	auto message = std::string{};
	message.reserve(4096);

	message.clear();
	message += "<<< Create SDL OpenGL context.";
	detail::sdl_log_eol(message);
	message += "Input parameters:";
	detail::sdl_log_eol(message);
	message += "  Window ptr: ";
	detail::sdl_log_xint_hex(reinterpret_cast<UInt>(&sdl_window), message);
	logger_.log_information(message);

	auto sdl_context = SdlGlContextUPtr{sdl_ensure_result(SDL_GL_CreateContext(&sdl_window))};

	attributes_.is_accelerated = get_attrib_bool(SDL_GL_ACCELERATED_VISUAL);
	attributes_.profile = map_profile(get_attrib(SDL_GL_CONTEXT_PROFILE_MASK));
	attributes_.major_version = get_attrib(SDL_GL_CONTEXT_MAJOR_VERSION);
	attributes_.minor_version = get_attrib(SDL_GL_CONTEXT_MINOR_VERSION);
	attributes_.multisample_buffer_count = get_attrib(SDL_GL_MULTISAMPLEBUFFERS);
	attributes_.multisample_sample_count = get_attrib(SDL_GL_MULTISAMPLESAMPLES);
	attributes_.red_bit_count = get_attrib(SDL_GL_RED_SIZE);
	attributes_.green_bit_count = get_attrib(SDL_GL_GREEN_SIZE);
	attributes_.blue_bit_count = get_attrib(SDL_GL_BLUE_SIZE);
	attributes_.alpha_bit_count = get_attrib(SDL_GL_ALPHA_SIZE);
	attributes_.depth_bit_count = get_attrib(SDL_GL_DEPTH_SIZE);

	sdl_context_.swap(sdl_context);

	message.clear();
	message += "Ptr: ";
	detail::sdl_log_xint_hex(reinterpret_cast<UInt>(sdl_context_.get()), message);
	detail::sdl_log_eol(message);
	message += "Effective attributes:";
	detail::sdl_log_eol(message);
	detail::sdl_log_gl_attributes(attributes_, message);
	message += ">>> SDL OpenGL context created.";
	logger_.log_information(message);
}
BSTONE_STATIC_THROW_NESTED_FUNC

SdlGlContext::~SdlGlContext()
{
	auto message = std::string{};
	message.reserve(128);

	message.clear();
	message += "Destroy SDL OpenGL context (ptr: ";
	detail::sdl_log_xint_hex(reinterpret_cast<UInt>(sdl_context_.get()), message);
	message += ')';
	logger_.log_information(message);
}

void* SdlGlContext::operator new(std::size_t count)
try
{
	return sdl_gl_context_pool.allocate(count);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void SdlGlContext::operator delete(void* ptr) noexcept
{
	sdl_gl_context_pool.deallocate(ptr);
}

const GlContextAttributes& SdlGlContext::do_get_attributes() const noexcept
{
	return attributes_;
}

GlContextProfile SdlGlContext::map_profile(int sdl_context_profile)
try
{
	switch (sdl_context_profile)
	{
		case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY: return GlContextProfile::compatibility;
		case SDL_GL_CONTEXT_PROFILE_CORE: return GlContextProfile::core;
		case SDL_GL_CONTEXT_PROFILE_ES: return GlContextProfile::es;
		default: BSTONE_STATIC_THROW("Unknown SDL context profile.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

int SdlGlContext::get_attrib(SDL_GLattr gl_attrib)
try
{
	auto gl_value = 0;
	sdl_ensure_result(SDL_GL_GetAttribute(gl_attrib, &gl_value));
	return gl_value;
}
BSTONE_STATIC_THROW_NESTED_FUNC

bool SdlGlContext::get_attrib_bool(SDL_GLattr gl_attrib)
try
{
	switch (get_attrib(gl_attrib))
	{
		case SDL_FALSE: return false;
		case SDL_TRUE: return true;
		default: BSTONE_STATIC_THROW("Invalid boolean value.");
	}
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace

// ==========================================================================

GlContextUPtr make_sdl_gl_context(Logger& logger, SDL_Window& sdl_window)
{
	return std::make_unique<SdlGlContext>(logger, sdl_window);
}

} // namespace sys
} // namespace bstone
