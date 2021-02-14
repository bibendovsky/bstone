/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
