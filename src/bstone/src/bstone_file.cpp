/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive.

#include "bstone_file.h"
#include <algorithm>

// ==========================================================================

namespace bstone {

File::File(File&& rhs) noexcept
	:
	handle_(rhs.handle_)
{
	rhs.handle_ = invalid_handle;
}

File& File::operator=(File&& rhs) noexcept
{
	std::swap(handle_, rhs.handle_);
	return *this;
}

bool File::lock(FileLockType lock_type) const
{
	switch (lock_type)
	{
		case file_lock_shared: return lock_shared();
		case file_lock_exclusive: return lock_exclusive();
		default: return false;
	}
}

void File::swap(File& file)
{
	std::swap(handle_, file.handle_);
}

// ==========================================================================

FileFlags operator|(FileFlags a, FileFlags b)
{
	return FileFlags(static_cast<int>(a) | static_cast<int>(b));
}

FileFlags& operator|=(FileFlags& a, FileFlags b)
{
	return a = a | b;
}

} // namespace bstone
