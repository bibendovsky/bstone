/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive.

#include "bstone_file.h"
#include "bstone_utility.h"

namespace bstone {

File::File(const char* file_name)
{
	open(file_name);
}

void File::open(const char* file_name)
{
	open(file_name, FileOpenMode::read);
}

void File::close()
{
	resource_.reset();
}

bool File::is_open() const
{
	return !resource_.is_empty();
}

} // namespace bstone
