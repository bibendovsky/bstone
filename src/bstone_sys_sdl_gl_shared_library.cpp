/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "SDL_video.h"

#include "bstone_exception.h"
#include "bstone_configurations.h"
#include "bstone_generic_pool_memory_resource.h"

#include "bstone_sys_sdl_exception.h"
#include "bstone_sys_sdl_gl_shared_library.h"

namespace bstone {
namespace sys {

namespace {

class SdlGlSharedLibrary final : public GlSharedLibrary
{
public:
	SdlGlSharedLibrary(const char* path);
	~SdlGlSharedLibrary() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	void* do_find_symbol(const char* name) noexcept override;

private:
	static MemoryResource& get_memory_resource();
};

} // namespace

// ==========================================================================

SdlGlSharedLibrary::SdlGlSharedLibrary(const char* path)
try {
	sdl_ensure_result(SDL_GL_LoadLibrary(path));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SdlGlSharedLibrary::~SdlGlSharedLibrary()
{
	SDL_GL_UnloadLibrary();
}

void* SdlGlSharedLibrary::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlGlSharedLibrary::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

void* SdlGlSharedLibrary::do_find_symbol(const char* name) noexcept
{
	return SDL_GL_GetProcAddress(name);
}

MemoryResource& SdlGlSharedLibrary::get_memory_resource()
{
	struct Initializer
	{
		Initializer(GenericPoolMemoryResource& generic_memory_pool)
		{
			generic_memory_pool.reserve(
				static_cast<std::intptr_t>(sizeof(SdlGlSharedLibrary)),
				sys_max_gl_shared_libraries,
				get_default_memory_resource());
		}
	};

	static GenericPoolMemoryResource generic_memory_pool{};
	static const Initializer initializer{generic_memory_pool};

	return generic_memory_pool;
}

// ==========================================================================

GlSharedLibraryUPtr make_sdl_gl_shared_library(const char* path)
{
	return std::make_unique<SdlGlSharedLibrary>(path);
}

} // namespace sys
} // namespace bstone
