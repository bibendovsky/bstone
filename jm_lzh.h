#ifndef _JM_LZH_H_
#define _JM_LZH_H_

extern void (*LZH_CompressDisplayVector)(unsigned long, unsigned long);
extern void (*LZH_DecompressDisplayVector)(unsigned long, unsigned long);



//===========================================================================
//
//											PROTOTYPES
//
//===========================================================================


boolean LZH_Startup(void);
void LZH_Shutdown(void);
long LZH_Compress(void far *infile, void far *outfile,unsigned long DataLength,unsigned PtrTypes);
long LZH_Decompress(void far *infile, void far *outfile, unsigned long OrginalLength, unsigned long CompressLength, unsigned PtrTypes);

#endif