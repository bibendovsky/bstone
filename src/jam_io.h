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


//
// UNIT : JAM_IO.h
//
// FUNCTION : General defines, prototypes, typedefs used in all the
// supported compression techniques used in JAMPAK Ver x.x
//
//


#ifndef BSTONE_JAM_IO_INCLUDED
#define BSTONE_JAM_IO_INCLUDED


#include <cstdint>


// =========================================================================
//
// FILE CHUNK IDs
//
// NOTE: The only reason for changing from COMP to CMP1 and having multi
// comp header structs is for downward compatibility.
//

#define COMP ("COMP") // Comp type is ct_LZW ALWAYS!
#define JAMP ("JAMP") // New Version different from SOFTDISK.


//
//      COMPRESSION TYPES
//
enum ct_TYPES
{
	ct_NONE = 0, // No compression - Just data..Rarely used!
	ct_LZW, // LZW data compression
	ct_LZH,

}; // ct_TYPES

//
//      FILE CHUNK HEADER FORMATS
//

struct COMPStruct
{
	std::uint32_t DecompLen;
};


struct JAMPHeader
{
	std::uint32_t OrginalLen; // Original FileLength of compressed Data.
	ct_TYPES CompType; // SEE: ct_TYPES above for list of pos.
	std::uint32_t CompressLen; // Length of data after compression (A MUST for LZHUFF!)
};


#endif // BSTONE_JAM_IO_INCLUDED
