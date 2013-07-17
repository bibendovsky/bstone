#include "id_heads.h"

#include <io.h>

#include "jm_io.h"
#include "jm_cio.h"
#include "jm_lzh.h"
#include "jm_error.h"

//--------------------------------------------------------------------------
// IO_FarRead()
//--------------------------------------------------------------------------
boolean IO_FarRead (int handle, byte far *dest, long length)
{
	unsigned readlen,nread;

	while (length)
	{
		if (length > 0xffff)
			readlen=0xffff;
		else
			readlen=length;

		_dos_read(handle,dest,readlen,&nread);
		if (nread != readlen)
			return(false);

		length -= readlen;
	}

	return(true);
}

//--------------------------------------------------------------------------
// IO_FarWrite()
//--------------------------------------------------------------------------
boolean IO_FarWrite (int handle, byte far *source, long length)
{
	unsigned writelen,nwritten;

	while (length)
	{
		if (length > 0xffff)
			writelen=0xffff;
		else
			writelen=length;

		_dos_write(handle,source,writelen,&nwritten);
		if (nwritten != writelen)
			return(false);

		length -= writelen;
	}

	return(true);
}

#if DEMOS_EXTERN

//--------------------------------------------------------------------------
// IO_WriteFile()
//--------------------------------------------------------------------------
boolean IO_WriteFile(char *filename, void far *ptr, long length)
{
	int handle;
	long size;

	handle = open(filename,O_CREAT | O_BINARY | O_WRONLY,
				S_IREAD | S_IWRITE | S_IFREG);

	if (handle == -1)
		return(false);

	if (!IO_FarWrite (handle,ptr,length))
	{
		close (handle);
		return(false);
	}
	close (handle);
	return(true);
}

#endif				 

//--------------------------------------------------------------------------
// IO_LoadFile()
//--------------------------------------------------------------------------
long IO_LoadFile (char *filename, memptr *dst)
{
	char buffer[5]={0,0,0,0,0};
	int handle;
	long size=0;

	if ((handle = open(filename,O_RDONLY | O_BINARY, S_IREAD)) == -1)
		return(size);

	read(handle,buffer,4);
	if (!strcmp(buffer,"JAMP"))
	{
		struct JAMPHeader head;

		read(handle,&head,sizeof(head));
		size = head.OriginalLen;
		switch (head.CompType)
		{
			case ct_LZH:
				LZH_Startup();
				MM_GetPtr(dst,head.OriginalLen);
				if (mmerror)
					return(0);
				LZH_Decompress((void far *)handle,*dst,size,head.CompressLen,SRC_FILE|DEST_MEM);
				LZH_Shutdown();
			break;

			case ct_LZW:
				IO_ERROR(IO_LOADFILE_NO_LZW);
			break;

			default:
				IO_ERROR(IO_LOADFILE_UNKNOWN);
			break;
		}
	}
	else
	{
		lseek(handle,0,SEEK_SET);
		size = filelength(handle);
		MM_GetPtr(dst,size);
		if (!IO_FarRead(handle,*dst,size))
		{
			close(handle);
			return(size);
		}
	}

	close(handle);

	return(size);
}

#if 0

//--------------------------------------------------------------------------
// IO_CopyFile()
//--------------------------------------------------------------------------
void IO_CopyFile(char *sFilename, char *dFilename)
{
	int sHandle,dHandle;
	unsigned length;

// Allocate memory for buffer.
//
	if ((sHandle = open(sFilename,O_RDONLY | O_BINARY, S_IREAD)) == -1)
		IO_ERROR(IO_COPYFILE_OPEN_SRC);

	if ((dHandle=open(dFilename,O_CREAT|O_RDWR|O_BINARY,S_IREAD|S_IWRITE))==-1)
		IO_ERROR(IO_COPYFILE_OPEN_DEST);

// Copy that file!
//
	IO_CopyHandle(sHandle,dHandle,-1);

// Close files.
//
	close(sHandle);
	close(dHandle);
}

#endif

//--------------------------------------------------------------------------
// IO_CopyHandle()
//--------------------------------------------------------------------------
void IO_CopyHandle(int sHandle, int dHandle, long num_bytes)
{
	extern boolean bombonerror;

	#define CF_BUFFER_SIZE 8192

	long fsize;
	memptr src;

	unsigned length;

// Allocate memory for buffer.
//
	MM_GetPtr(&src,CF_BUFFER_SIZE);
	if (num_bytes == -1)
		fsize=filelength(sHandle);
	else
		fsize=num_bytes;

// Copy that file!
//
	do
	{
	// Determine length to read/write.
	//
		if (fsize >= CF_BUFFER_SIZE)
			length = CF_BUFFER_SIZE;
		else
			length = fsize;

	// Read it, write it and decrement length.
	//
		IO_FarRead(sHandle,src,length);
		IO_FarWrite(dHandle,src,length);
		fsize -= length;
	}
	while (fsize);

// Free buffer.
//
	MM_FreePtr(&src);
}

