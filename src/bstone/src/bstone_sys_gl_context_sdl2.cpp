/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <string>
#include <type_traits>
#include "bstone_configurations.h"
#include "bstone_exception.h"
#include "bstone_generic_pool_resource.h"
#include "bstone_sys_detail_sdl2.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_gl_context_sdl2.h"

namespace bstone {
namespace sys {

namespace {

struct Sdl2GlContextDeleter
{
	void operator()(SDL_GLContext sdl_gl_context) const noexcept
	{
		SDL_GL_DeleteContext(sdl_gl_context);
	}
};

using Sdl2GlContextUPtr = std::unique_ptr<std::remove_pointer_t<SDL_GLContext>, Sdl2GlContextDeleter>;

// ==========================================================================

class Sdl2GlContext final : public GlContext
{
public:
	Sdl2GlContext(Logger& logger, SDL_Window& sdl_window);
	Sdl2GlContext(const Sdl2GlContext&) = delete;
	Sdl2GlContext& operator=(const Sdl2GlContext&) = delete;
	~Sdl2GlContext() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	Sdl2GlContextUPtr sdl_context_{};
	GlContextAttributes attributes_{};

private:
	const GlContextAttributes& do_get_attributes() const noexcept override;

private:
	static MemoryResource& get_memory_resource();

	static GlContextProfile map_profile(int sdl_context_profile);
	static int get_attrib(SDL_GLattr gl_attrib);
	static bool get_attrib_bool(SDL_GLattr gl_attrib);
};

// ==========================================================================

Sdl2GlContext::Sdl2GlContext(Logger& logger, SDL_Window& sdl_window)
try
	:
	logger_{logger}
{
	auto message = std::string{};
	message.reserve(4096);

	message.clear();
	message += "<<< Create SDL OpenGL context.";
	detail::sdl2_log_eol(message);
	message += "Input parameters:";
	detail::sdl2_log_eol(message);
	message += "  Window ptr: ";
	detail::sdl2_log_xint_hex(reinterpret_cast<std::uintptr_t>(&sdl_window), message);
	logger_.log_information(message.c_str());

	auto sdl_context = Sdl2GlContextUPtr{sdl2_ensure_result(SDL_GL_CreateContext(&sdl_window))};

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
	detail::sdl2_log_xint_hex(reinterpret_cast<std::uintptr_t>(sdl_context_.get()), message);
	detail::sdl2_log_eol(message);
	message += "Effective attributes:";
	detail::sdl2_log_eol(message);
	detail::sdl2_log_gl_attributes(attributes_, message);
	message += ">>> SDL OpenGL context created.";
	logger_.log_information(message.c_str());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2GlContext::~Sdl2GlContext()
{
	auto message = std::string{};
	message.reserve(128);

	message.clear();
	message += "Destroy SDL OpenGL context (ptr: ";
	detail::sdl2_log_xint_hex(reinterpret_cast<std::uintptr_t>(sdl_context_.get()), message);
	message += ')';
	logger_.log_information(message.c_str());
}

void* Sdl2GlContext::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2GlContext::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

const GlContextAttributes& Sdl2GlContext::do_get_attributes() const noexcept
{
	return attributes_;
}

MemoryResource& Sdl2GlContext::get_memory_resource()
{
	struct Initializer
	{
		Initializer(GenericPoolResource& generic_memory_pool)
		{
			generic_memory_pool.reserve(
				static_cast<std::intptr_t>(sizeof(Sdl2GlContext)),
				sys_max_gl_contexts,
				get_default_memory_resource());
		}
	};

	static GenericPoolResource generic_memory_pool{};
	static const Initializer initializer{generic_memory_pool};

	return generic_memory_pool;
}

GlContextProfile Sdl2GlContext::map_profile(int sdl_context_profile)
try {
	switch (sdl_context_profile)
	{
		case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY: return GlContextProfile::compatibility;
		case SDL_GL_CONTEXT_PROFILE_CORE: return GlContextProfile::core;
		case SDL_GL_CONTEXT_PROFILE_ES: return GlContextProfile::es;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown SDL context profile.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int Sdl2GlContext::get_attrib(SDL_GLattr gl_attrib)
try {
	auto gl_value = 0;
	sdl2_ensure_result(SDL_GL_GetAttribute(gl_attrib, &gl_value));
	return gl_value;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool Sdl2GlContext::get_attrib_bool(SDL_GLattr gl_attrib)
try {
	switch (get_attrib(gl_attrib))
	{
		case SDL_FALSE: return false;
		case SDL_TRUE: return true;
		default: BSTONE_THROW_STATIC_SOURCE("Invalid boolean value.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ==========================================================================

GlContextUPtr make_sdl2_gl_context(Logger& logger, SDL_Window& sdl_window)
{
	return std::make_unique<Sdl2GlContext>(logger, sdl_window);
}

} // namespace sys
} // namespace bstone
