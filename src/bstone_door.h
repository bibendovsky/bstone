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

#ifndef BSTONE_DOOR_INCLUDED
#define BSTONE_DOOR_INCLUDED

enum doortype
{
	// LOCKED DOORS

	L_METAL,
	L_METAL_SHADE,

	L_BIO,
	L_BIO_SHADE,

	L_ELEVATOR,
	L_ELEVATOR_SHADE,

	L_SPACE,
	L_SPACE_SHADE,

	L_PRISON,
	L_PRISON_SHADE,

	L_HIGH_SECURITY,
	L_HIGH_SECURITY_SHADE,

	L_ENTER_ONLY,
	L_ENTER_ONLY_SHADE,

	L_HIGH_TECH,
	L_HIGH_TECH_SHADE,


	// UNLOCKED DOORS

	UL_METAL,
	UL_METAL_SHADE,

	UL_BIO,
	UL_BIO_SHADE,

	UL_ELEVATOR,
	UL_ELEVATOR_SHADE,

	UL_SPACE,
	UL_SPACE_SHADE,

	UL_PRISON,
	UL_PRISON_SHADE,

	UL_HIGH_SECURITY,
	UL_HIGH_SECURITY_SHADE,

	UL_ENTER_ONLY,
	UL_ENTER_ONLY_SHADE,

	UL_HIGH_TECH,
	UL_HIGH_TECH_SHADE,


	// MISC DOORS

	NOEXIT,
	NOEXIT_SHADE,

	STEEL_JAM,
	STEEL_JAM_SHADE,

	SPACE_JAM,
	SPACE_JAM_SHADE,

	OFFICE_JAM,
	OFFICE_JAM_SHADE,

	BIO_JAM,
	BIO_JAM_SHADE,

	SPACE_JAM_2,
	SPACE_JAM_2_SHADE,

	// END OF DOOR LIST

	NUMDOORTYPES,
}; // doortype

int door_get_page_base_index();

#define DOORWALL door_get_page_base_index()

#endif // !BSTONE_DOOR_INCLUDED
