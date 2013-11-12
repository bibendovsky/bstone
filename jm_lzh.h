#ifndef _JM_LZH_H_
#define _JM_LZH_H_


#include "SDL.h"


extern void (*LZH_CompressDisplayVector)(Uint32, Uint32);
extern void (*LZH_DecompressDisplayVector)(Uint32, Uint32);



//===========================================================================
//
//											PROTOTYPES
//
//===========================================================================


bool LZH_Startup(void);
void LZH_Shutdown(void);

int LZH_Compress(
    const void* infile,
    void* outfile,
    Uint32 DataLength);

int LZH_Decompress(
    const void* infile,
    void* outfile,
    Uint32 OrginalLength,
    Uint32 CompressLength);

#endif
