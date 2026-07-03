/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_OAL_LOADER_INCLUDED
#define BSTONE_OAL_LOADER_INCLUDED

#include "bstone_oal_symbols.h"
#include <memory>

namespace bstone {

class OalLoader
{
public:
	OalLoader() = default;
	virtual ~OalLoader() = default;

	virtual void load_alc_symbols() = 0;
	virtual void load_al_symbols() = 0;
	virtual void load_efx_symbols() = 0;
};

// =====================================

using OalLoaderUPtr = std::unique_ptr<OalLoader>;

OalLoaderUPtr make_oal_loader(const char* shared_library_path);

} // namespace bstone

#endif // BSTONE_OAL_LOADER_INCLUDED
