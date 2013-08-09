#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <alloc.h>
#include <fcntl.h>
#include <dos.h>
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
			*((char *)*(char **)outfile)++ = data;
		break;
	}

	return(returnval);

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
			returnval = (Uint8)*((char *)*(char **)infile)++;
		break;
	}

	return(returnval);
}



