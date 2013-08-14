#ifndef _JM_IO_H_
#define _JM_IO_H_

//-------------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------------
boolean IO_FarRead(int handle, void* dest, int length);
boolean IO_FarWrite(int handle, const void* source, int length);
boolean IO_ReadFile(const char* filename, void** ptr);
boolean IO_WriteFile(const char* filename, const void* ptr, int length);
int IO_LoadFile (const char* filename, void** dst);
void IO_CopyFile(const char* sFilename, const char* dFilename);
void IO_CopyHandle(int sHandle, int dHandle, int num_bytes);

#endif
