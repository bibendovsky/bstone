/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Project's version.

#ifndef BSTONE_VERSION_INCLUDED
#define BSTONE_VERSION_INCLUDED

namespace bstone {

struct Version
{
	int major{};
	int minor{};
	int patch{};
	const char* pre_release{};
	const char* git_hash{};
	const char* string{};
	const char* string_short{};
};

// ==========================================================================

const Version& get_version() noexcept;

} // namespace bstone

#endif // BSTONE_VERSION_INCLUDED
