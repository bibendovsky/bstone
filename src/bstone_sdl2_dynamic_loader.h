#ifndef BSTONE_SDL2_DYNAMIC_LOADER_INCLUDED
#define BSTONE_SDL2_DYNAMIC_LOADER_INCLUDED


#include "bstone_dynamic_loader.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class Sdl2DynamicLoader final :
	public DynamicLoader
{
public:
	Sdl2DynamicLoader(
		const char* shared_library_path_name);

	~Sdl2DynamicLoader() override;


	void* resolve(
		const char* symbol_name) noexcept override;


private:
	void* sdl2_handle_{};
}; // Sdl2DynamicLoader

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_SDL2_DYNAMIC_LOADER_INCLUDED
