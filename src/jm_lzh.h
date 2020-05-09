/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#ifndef BSTONE_JM_LZH_INCLUDED
#define BSTONE_JM_LZH_INCLUDED


#include <cstdint>


extern void(*LZH_CompressDisplayVector)(std::uint32_t, std::uint32_t);
extern void(*LZH_DecompressDisplayVector)(
	std::uint32_t,
	std::uint32_t);



// ===========================================================================
//
// PROTOTYPES
//
// ===========================================================================


bool LZH_Startup();
void LZH_Shutdown();

int LZH_Compress(
	const void* infile,
	void* outfile,
	std::uint32_t DataLength);

int LZH_Decompress(
	const void* infile,
	void* outfile,
	std::uint32_t OrginalLength,
	std::uint32_t CompressLength);


#endif // BSTONE_JM_LZH_INCLUDED
