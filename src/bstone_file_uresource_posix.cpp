/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File's unique resource (POSIX).

#if !defined(_WIN32)

#include <cassert>

#include <unistd.h>

#include "bstone_file_uresource.h"

namespace bstone {

FileUResourceHandle FileUResourceEmptyValue::operator()() const noexcept
{
	return -1;
}

void FileUResourceDeleter::operator()(FileUResourceHandle handle) const
{
#if !defined(NDEBUG)
	const auto posix_result =
#endif
		close(handle);
#if !defined(NDEBUG)
	assert(posix_result == 0);
#endif
}

} // namespace bstone

#endif // _WIN32
