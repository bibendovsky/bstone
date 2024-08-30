/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Information about the source code: file name, line number, function name.

#include "bstone_source_location.h"

#include "bstone_assert.h"

namespace bstone {

SourceLocation::SourceLocation(const char* file_name, int line, const char* function_name)
	:
	line_{line >= 0 ? line : 0},
	file_name_{file_name != nullptr ? file_name : ""},
	function_name_{function_name != nullptr ? function_name : ""}
{
	BSTONE_ASSERT(file_name != nullptr);
	BSTONE_ASSERT(line >= 0);
	BSTONE_ASSERT(function_name != nullptr);
}

} // namespace bstone
