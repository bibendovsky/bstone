/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "3d_def.h"

namespace {

// Offsets of variables in data segment.

constexpr auto objlist_offset = 0xFFFF - MAXACTORS;
constexpr auto statobjlist_offset = 0xFFFF - MAXSTATS;
constexpr auto doorobjlist_offset = 0xFFFF - MAXDOORS;

} // namespace

objtype* ui16_to_actor(std::uint16_t value)
{
	const auto index = value - objlist_offset;

	if (index < 0 || index >= MAXACTORS)
	{
		return nullptr;
	}

	return &objlist[index];
}

std::uint16_t actor_to_ui16(const objtype* actor)
{
	const auto index = actor - objlist;

	if (index < 0 || index >= MAXACTORS)
	{
		return 0;
	}

	return static_cast<std::uint16_t>(index + objlist_offset);
}

statobj_t* ui16_to_static_object(std::uint16_t value)
{
	const auto index = value - statobjlist_offset;

	if (index < 0 || index >= MAXSTATS)
	{
		return nullptr;
	}

	return &statobjlist[index];
}

std::uint16_t static_object_to_ui16(const statobj_t* static_object)
{
	const auto index = static_object - statobjlist.data();

	if (index < 0 || index >= MAXSTATS)
	{
		return 0;
	}

	return static_cast<std::uint16_t>(index + statobjlist_offset);
}

doorobj_t* ui16_to_door_object(std::uint16_t value)
{
	const auto index = value - doorobjlist_offset;

	if (index < 0 || index >= MAXDOORS)
	{
		return nullptr;
	}

	return &doorobjlist[index];
}

std::uint16_t door_object_to_ui16(const doorobj_t* door_object)
{
	const auto index = door_object - doorobjlist;

	if (index < 0 || index >= MAXDOORS)
	{
		return 0;
	}

	return static_cast<std::uint16_t>(index + doorobjlist_offset);
}
