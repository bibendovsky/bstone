#ifndef BSTONE_OAL_LOADER_INCLUDED
#define BSTONE_OAL_LOADER_INCLUDED


#include <memory>

#include "bstone_oal_symbols.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalLoader
{
public:
	OalLoader() noexcept;

	virtual ~OalLoader();


	virtual const OalSymbols& get_symbols() const noexcept = 0;
}; // OalLoader

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using OalLoaderUPtr = std::unique_ptr<OalLoader>;


OalLoaderUPtr make_oal_loader(
	const char* shared_library_path_name);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_OAL_LOADER_INCLUDED
