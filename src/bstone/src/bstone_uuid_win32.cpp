/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Universally unique identifier (UUID) (WIN32).
//
// Supported formats:
// - "AAAAAAAABBBBCCCCDDDDEEEEEEEEEEEE"
// - "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE"
// - "{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}"

#include "bstone_uuid.h"

#ifdef _WIN32

#include <objbase.h>

#include "bstone_exception.h"

namespace bstone {

Uuid Uuid::generate()
{
	auto guid = GUID{};

	if (FAILED(CoCreateGuid(&guid)))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to generate UUID.");
	}

	return Uuid{guid.Data1, guid.Data2, guid.Data3, guid.Data4};
}

} // namespace bstone

#endif // _WIN32
