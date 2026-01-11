/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive.

#include "bstone_file.h"
#include "bstone_assert.h"
#include <algorithm>
#include <memory>

// ==========================================================================

namespace bstone {

File::File(File&& rhs) noexcept
	:
	handle_{rhs.handle_},
	is_readable_{rhs.is_readable_}
{
	rhs.handle_ = nullptr;
}

File& File::operator=(File&& rhs) noexcept
{
	BSTONE_ASSERT(std::addressof(rhs) != this);
	close();
	handle_ = rhs.handle_;
	rhs.handle_ = nullptr;
	is_readable_ = rhs.is_readable_;
	return *this;
}

void File::swap(File& file)
{
	std::swap(handle_, file.handle_);
	std::swap(is_readable_, file.is_readable_);
}

// ======================================

FileFlags operator|(FileFlags a, FileFlags b)
{
	return FileFlags(static_cast<int>(a) | static_cast<int>(b));
}

FileFlags& operator|=(FileFlags& a, FileFlags b)
{
	return a = a | b;
}

} // namespace bstone
