/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Project's version.

#include "bstone_version.h"

namespace bstone {

namespace {

static Version version_impl
{
#include "bstone_version_.h"
};

} // namespace

const Version& get_version() noexcept
{
	return version_impl;
}

} // namespace bstone
