/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_OAL_LOADER_INCLUDED
#define BSTONE_OAL_LOADER_INCLUDED

#include <memory>
#include "bstone_oal_symbols.h"

namespace bstone
{

class OalLoader
{
public:
	OalLoader() noexcept;
	virtual ~OalLoader();

	virtual void load_alc_symbols(OalAlSymbols& al_symbols) = 0;
	virtual void load_al_symbols(OalAlSymbols& al_symbols) = 0;
	virtual void load_efx_symbols(OalAlSymbols& al_symbols) = 0;
}; // OalLoader

// ==========================================================================

using OalLoaderUPtr = std::unique_ptr<OalLoader>;

OalLoaderUPtr make_oal_loader(const char* shared_library_path);

} // bstone

#endif // !BSTONE_OAL_LOADER_INCLUDED
