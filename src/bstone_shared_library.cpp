/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_shared_library.h"

namespace bstone {

bool SharedLibrary::is_open() const noexcept
{
	return handle_ != nullptr;
}

SharedLibrary::SharedLibrary(const char* file_path)
{
	open(file_path);
}

void SharedLibrary::close()
{
	handle_ = nullptr;
}

void SharedLibrary::ensure_is_open() const
{
	if (!is_open())
	{
		BSTONE_THROW_STATIC_SOURCE("Closed shared library.");
	}
}

} // namespace bstone
