/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Information about the source code: file name, line number, function name.

#include <cassert>

#include "bstone_source_location.h"

namespace bstone {

SourceLocation::SourceLocation(const char* file_name, int line, const char* function_name)
	:
	line_{line >= 0 ? line : 0},
	file_name_{file_name != nullptr ? file_name : ""},
	function_name_{function_name != nullptr ? function_name : ""}
{
	assert(file_name != nullptr);
	assert(line >= 0);
	assert(function_name != nullptr);
}

} // namespace bstone
