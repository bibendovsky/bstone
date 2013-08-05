#ifndef _JM_IO_H_
#define _JM_IO_H_

//-------------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------------
boolean IO_FarRead (int handle, byte *dest, long length);
boolean IO_FarWrite (int handle, byte *source, long length);
boolean IO_ReadFile (char *filename, void** ptr);
boolean IO_WriteFile(char *filename, void *ptr, long length);
long IO_LoadFile (char *filename, void** dst);
void IO_CopyFile(char *sFilename, char *dFilename);
void IO_CopyHandle(int sHandle, int dHandle, long num_bytes);

#endif
