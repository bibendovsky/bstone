#ifndef _JM_IO_H_
#define _JM_IO_H_

//-------------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------------
boolean IO_FarRead (int handle, byte far *dest, long length);
boolean IO_FarWrite (int handle, byte far *source, long length);
boolean IO_ReadFile (char *filename, memptr *ptr);
boolean IO_WriteFile(char *filename, void far *ptr, long length);
long IO_LoadFile (char *filename, memptr *dst);
void IO_CopyFile(char *sFilename, char *dFilename);
void IO_CopyHandle(int sHandle, int dHandle, long num_bytes);

#endif
