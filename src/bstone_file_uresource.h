/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File's unique resource.

#if !defined(BSTONE_FILE_URESOURCE_INCLUDED)
#define BSTONE_FILE_URESOURCE_INCLUDED

#include "bstone_unique_resource.h"

namespace bstone {

using FileUResourceHandle =
#if defined(_WIN32)
	void*
#else
	int
#endif
;

struct FileUResourceEmptyValue
{
	FileUResourceHandle operator()() const noexcept;
};

struct FileUResourceDeleter
{
	void operator()(FileUResourceHandle handle) const;
};

using FileUResource = UniqueResource<
	FileUResourceHandle,
	FileUResourceDeleter,
	FileUResourceEmptyValue>;

} // namespace bstone

#endif // BSTONE_FILE_URESOURCE_INCLUDED
