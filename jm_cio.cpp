/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


#include "jm_cio.h"


// ----------------------------------------------------------------------------
//
// PTR/PTR COMPRESSION ROUTINES
//
//
// ----------------------------------------------------------------------------



// ---------------------------------------------------------------------------
// CIO_WritePtr()  -- Outputs data to a particular ptr type
//
//      PtrType MUST be of type DEST_TYPE.
//
// NOTE : For PtrTypes DEST_MEM a ZERO (0) is always returned.
//
// ---------------------------------------------------------------------------

char CIO_WritePtr(
    void*& buffer,
    uint8_t value)
{
    uint8_t*& buffer_ref = reinterpret_cast<uint8_t*&>(buffer);
    buffer_ref[0] = value;
    ++buffer_ref;
    return '\0';
}

// ---------------------------------------------------------------------------
// CIO_ReadPtr()  -- Reads data from a particular ptr type
//
//      PtrType MUST be of type SRC_TYPE.
//
// RETURNS :
//              The char read in or EOF for SRC_FFILE type of reads.
//
//
// ---------------------------------------------------------------------------

int16_t CIO_ReadPtr(
    const void*& buffer)
{
    const uint8_t*& buffer_ref = reinterpret_cast<const uint8_t*&>(buffer);
    uint8_t value = buffer_ref[0];
    ++buffer_ref;
    return value;
}
