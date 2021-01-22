#ifndef BSTONE_DYNAMIC_LOADER_INCLUDED
#define BSTONE_DYNAMIC_LOADER_INCLUDED


#include <memory>


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class DynamicLoader
{
public:
	DynamicLoader() noexcept;

	virtual ~DynamicLoader();


	virtual void* resolve(
		const char* symbol_name) noexcept = 0;
}; // DynamicLoader

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using DynamicLoaderUPtr = std::unique_ptr<DynamicLoader>;


DynamicLoaderUPtr make_dynamic_loader(
	const char* shared_library_path_name);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_DYNAMIC_LOADER_INCLUDED
