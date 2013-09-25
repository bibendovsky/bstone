#include <cstdio>
#include <io.h>

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

// FIXME
#if 0
char CIO_WritePtr(Sint32 outfile, Uint8 data, Uint16 PtrType)
{
	Sint16 returnval = 0;

	switch (PtrType & DEST_TYPES)
	{
		case DEST_FILE:
			write(*(Sint16 *)outfile,(char *)&data,1);
		break;

		case DEST_FFILE:
			returnval = putc(data, *(FILE **)outfile);
		break;

		case DEST_IMEM:
			printf("CIO_WritePtr - unsupported ptr type\n");
			exit(0);
		break;

		case DEST_MEM:
// FIXME
#if 0
			*((char *)*(char **)outfile)++ = data;
#endif // 0

            (reinterpret_cast<char**>(outfile)[0]++)[0] = data;
		break;
	}

	return(returnval);

}
#endif // 0

// BBi
char CIO_WritePtr(
    void*& buffer,
    Uint8 value)
{
    Uint8*& buffer_ref = reinterpret_cast<Uint8*&>(buffer);
    buffer_ref[0] = value;
    ++buffer_ref;
    return '\0';
}
// BBi

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

// FIXME
#if 0
Sint16 CIO_ReadPtr(Sint32 infile, Uint16 PtrType)
{
	Sint16 returnval = 0;

	switch (PtrType & SRC_TYPES)
	{
		case SRC_FILE:
			read(*(Sint16 *)infile,(char *)&returnval,1);
		break;

		case SRC_FFILE:
			returnval = getc(*(FILE **)infile);
		break;

#if 0
		case SRC_BFILE:
			returnval = bio_readch((BufferedIO *)*(void **)infile);
		break;
#endif

//		case SRC_IMEM:
//			printf("CIO_WritePtr - unsupported ptr type\n");
//			exit(0);
//		break;

		case SRC_MEM:
// FIXME
#if 0
			returnval = (Uint8)*((char *)*(char **)infile)++;
#endif // 0

            returnval = (reinterpret_cast<const Uint8**>(infile)[0]++)[0];
		break;
	}

	return(returnval);
}
#endif // 0

// BBi
Sint16 CIO_ReadPtr(
    const void*& buffer)
{
    const Uint8*& buffer_ref = reinterpret_cast<const Uint8*&>(buffer);
    Uint8 value = buffer_ref[0];
    ++buffer_ref;
    return value;
}
// BBi
