#ifndef _JM_LZH_H_
#define _JM_LZH_H_

extern void (*LZH_CompressDisplayVector)(Uint32, Uint32);
extern void (*LZH_DecompressDisplayVector)(Uint32, Uint32);



//===========================================================================
//
//											PROTOTYPES
//
//===========================================================================


boolean LZH_Startup(void);
void LZH_Shutdown(void);
Sint32 LZH_Compress(void *infile, void *outfile,Uint32 DataLength,Uint16 PtrTypes);
Sint32 LZH_Decompress(void *infile, void *outfile, Uint32 OrginalLength, Uint32 CompressLength, Uint16 PtrTypes);

#endif