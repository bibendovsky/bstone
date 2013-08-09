#ifndef _JM_IO_H_
#define _JM_IO_H_

//-------------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------------
boolean IO_FarRead (int handle, Uint8 *dest, Sint32 length);
boolean IO_FarWrite (int handle, Uint8 *source, Sint32 length);
boolean IO_ReadFile (char *filename, void** ptr);
boolean IO_WriteFile(char *filename, void *ptr, Sint32 length);
Sint32 IO_LoadFile (char *filename, void** dst);
void IO_CopyFile(char *sFilename, char *dFilename);
void IO_CopyHandle(int sHandle, int dHandle, Sint32 num_bytes);

#endif
