/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "3d_def.h"


//
// Offsets of variables in data segment.
//

const int OBJLIST_OFFSET = 0xFFFF - MAXACTORS;
const int STATOBJLIST_OFFSET = 0xFFFF - MAXSTATS;
const int DOOROBJLIST_OFFSET = 0xFFFF - MAXDOORS;


objtype* ui16_to_actor(
	std::uint16_t value)
{
	int index = value - OBJLIST_OFFSET;

	if (index < 0)
	{
		return nullptr;
	}

	if (index >= MAXACTORS)
	{
		return nullptr;
	}

	return &objlist[index];
}

std::uint16_t actor_to_ui16(
	const objtype* actor)
{
	auto index = actor - objlist;

	if (index < 0)
	{
		return 0;
	}

	if (index >= MAXACTORS)
	{
		return 0;
	}

	return static_cast<std::uint16_t>(index + OBJLIST_OFFSET);
}

statobj_t* ui16_to_static_object(
	std::uint16_t value)
{
	int index = value - STATOBJLIST_OFFSET;

	if (index < 0)
	{
		return nullptr;
	}

	if (index >= MAXSTATS)
	{
		return nullptr;
	}

	return &statobjlist[index];
}

std::uint16_t static_object_to_ui16(
	const statobj_t* static_object)
{
	auto index = static_object - statobjlist.data();

	if (index < 0)
	{
		return 0;
	}

	if (index >= MAXSTATS)
	{
		return 0;
	}

	return static_cast<std::uint16_t>(index + STATOBJLIST_OFFSET);
}

doorobj_t* ui16_to_door_object(
	std::uint16_t value)
{
	int index = value - DOOROBJLIST_OFFSET;

	if (index < 0)
	{
		return nullptr;
	}

	if (index >= MAXDOORS)
	{
		return nullptr;
	}

	return &doorobjlist[index];
}

std::uint16_t door_object_to_ui16(
	const doorobj_t* door_object)
{
	auto index = door_object - doorobjlist;

	if (index < 0)
	{
		return 0;
	}

	if (index >= MAXDOORS)
	{
		return 0;
	}

	return static_cast<std::uint16_t>(index + DOOROBJLIST_OFFSET);
}
