#include "jm_cio.h"


//----------------------------------------------------------------------------
//
//	 						PTR/PTR COMPRESSION ROUTINES
//
//
//----------------------------------------------------------------------------



//---------------------------------------------------------------------------
// CIO_WritePtr()  -- Outputs data to a particular ptr type
//
//	PtrType MUST be of type DEST_TYPE.
//
// NOTE : For PtrTypes DEST_MEM a ZERO (0) is always returned.
//
//---------------------------------------------------------------------------

char CIO_WritePtr(
    void*& buffer,
    Uint8 value)
{
    Uint8*& buffer_ref = reinterpret_cast<Uint8*&>(buffer);
    buffer_ref[0] = value;
    ++buffer_ref;
    return '\0';
}

//---------------------------------------------------------------------------
// CIO_ReadPtr()  -- Reads data from a particular ptr type
//
//	PtrType MUST be of type SRC_TYPE.
//
// RETURNS :
//		The char read in or EOF for SRC_FFILE type of reads.
//
//
//---------------------------------------------------------------------------

Sint16 CIO_ReadPtr(
    const void*& buffer)
{
    const Uint8*& buffer_ref = reinterpret_cast<const Uint8*&>(buffer);
    Uint8 value = buffer_ref[0];
    ++buffer_ref;
    return value;
}
