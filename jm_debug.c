#include "3D_DEF.H"

#include "jm_error.h"
#include "jm_debug.h"

#pragma	hdrstop

#ifdef __MPRINTF__

static char x=0;
static char y=0;
static char far *video = MK_FP(0xb000,0x0000);

//-------------------------------------------------------------------------
// mclear()
//-------------------------------------------------------------------------
void mclear(void)
{
	short length = 80*25*2;

	video = MK_FP(0xb000,0x0000);
	while (length--)
		*(short *)video++ = 0x0f20;

	x=y=0;
	video = MK_FP(0xb000,0x0000);
}

//-------------------------------------------------------------------------
// mprintf()
//-------------------------------------------------------------------------
void mprintf(char *msg, ...)
{
	char buffer[100],*ptr;

	va_list(ap);

	va_start(ap,msg);

	vsprintf(buffer,msg,ap);

	ptr = buffer;
	while (*ptr)
	{
		switch (*ptr)
		{
			case '\n':
				if (y >= 23)
				{
					video -= (x<<1);
					_fmemcpy(MK_FP(0xb000,0x0000),MK_FP(0xb000,0x00a0),3840);
				}
				else
				{
					y++;
					video += ((80-x)<<1);
				}
				x=0;
			break;

			default:
				*video = *ptr;
				video[1] = 15;
				video += 2;
				x++;
			break;
		}
		ptr++;
	}

	va_end(ap);
}

//-------------------------------------------------------------------------
// fmprint()
//-------------------------------------------------------------------------
void fmprint(char far *text)
{
	while (*text)
		mprintf("%c",*text++);
}

#endif

