/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Universally unique identifier (UUID) (Unix).
//
// Supported formats:
// - "AAAAAAAABBBBCCCCDDDDEEEEEEEEEEEE"
// - "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE"
// - "{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}"

// TODO
// Rewrite with "File" when non-regular files will be supported.

#include "bstone_uuid.h"

#ifndef _WIN32

#include <fcntl.h>
#include <unistd.h>

#include "bstone_exception.h"
#include "bstone_file.h"

namespace bstone {

Uuid Uuid::generate()
try {
	auto file = FileUResource{::open("/dev/urandom", O_RDONLY)};

	if (file.is_empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to open \"/dev/urandom\".");
	}

	auto uuid = Uuid{};
	auto& uuid_value = uuid.get_value();

	if (::read(file.get(), uuid_value.get_data(), uuid_value_size) != uuid_value_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to read random bytes.");
	}

	// Set version 4.
	auto new_version = uuid_value[6];
	new_version &= 0B0000'1111;
	new_version |= 0B0100'0000;
	uuid_value[6] = new_version;

	// Set variant 1.
	auto new_variant = uuid_value[8];
	new_variant &= 0B00'111111;
	new_variant |= 0B10'000000;
	uuid_value[8] = new_variant;

	return uuid;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone

#endif // _WIN32
