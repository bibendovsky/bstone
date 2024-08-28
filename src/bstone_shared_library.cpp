/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_shared_library.h"

#include "bstone_exception.h"

namespace bstone {

SharedLibrary::SharedLibrary() noexcept = default;

SharedLibrary::SharedLibrary(const char* file_path)
{
	open(file_path);
}

bool SharedLibrary::is_open() const noexcept
{
	return handle_ != nullptr;
}

void SharedLibrary::open(const char* file_path)
{
	if (!try_open(file_path))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to load a shared library.");
	}
}

void SharedLibrary::close() noexcept
{
	handle_ = nullptr;
}

void SharedLibrary::swap(SharedLibrary& rhs) noexcept
{
	handle_.swap(rhs.handle_);
}

} // namespace bstone
