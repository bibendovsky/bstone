#include "bstone_sdl2_dynamic_loader.h"

#include <string>

#include "SDL_loadso.h"

#include "bstone_sdl2_exception.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Sdl2DynamicLoader::Sdl2DynamicLoader(
	const char* shared_library_path_name)
{
	Sdl2EnsureResult{sdl2_handle_ = SDL_LoadObject(shared_library_path_name)};
}

Sdl2DynamicLoader::~Sdl2DynamicLoader()
{
	SDL_UnloadObject(sdl2_handle_);
}

void* Sdl2DynamicLoader::resolve(
	const char* symbol_name) noexcept
{
	return SDL_LoadFunction(sdl2_handle_, symbol_name);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

DynamicLoaderUPtr make_dynamic_loader(
	const char* shared_library_path_name)
{
	return std::make_unique<Sdl2DynamicLoader>(shared_library_path_name);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
