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
char CIO_WritePtr(long outfile, unsigned char data, unsigned short PtrType)
{
	short returnval = 0;

	switch (PtrType & DEST_TYPES)
	{
		case DEST_FILE:
			write(*(short *)outfile,(char *)&data,1);
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
short CIO_ReadPtr(long infile, unsigned short PtrType)
{
	short returnval = 0;

	switch (PtrType & SRC_TYPES)
	{
		case SRC_FILE:
			read(*(short *)infile,(char *)&returnval,1);
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
			returnval = (unsigned char)*((char *)*(char **)infile)++;
		break;
	}

	return(returnval);
}



