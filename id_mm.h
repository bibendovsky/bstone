/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
============================================================== */


#ifndef ID_MM_H
#define ID_MM_H


#if 0
// ID_MM.H

#ifndef __ID_CA__

#define __ID_CA__

#define SAVENEARHEAP	0x400		// space to leave in data segment
#define SAVEFARHEAP		0			// space to leave in far heap

#define	BUFFERSIZE		0x1000		// miscelanious, allways available buffer

#define MAXBLOCKS		600


//--------

#define	EMS_INT			0x67

#define	EMS_STATUS		0x40
#define	EMS_GETFRAME	0x41
#define	EMS_GETPAGES	0x42
#define	EMS_ALLOCPAGES	0x43
#define	EMS_MAPPAGE		0x44
#define	EMS_FREEPAGES	0x45
#define	EMS_VERSION		0x46

//--------

#define	XMS_INT			0x2f
#define	XMS_CALL(v)		_AH = (v);\
						asm call [DWORD PTR XMSDriver]

#define	XMS_VERSION		0x00

#define	XMS_ALLOCHMA	0x01
#define	XMS_FREEHMA		0x02

#define	XMS_GENABLEA20	0x03
#define	XMS_GDISABLEA20	0x04
#define	XMS_LENABLEA20	0x05
#define	XMS_LDISABLEA20	0x06
#define	XMS_QUERYA20	0x07

#define	XMS_QUERYFREE	0x08
#define	XMS_ALLOC		0x09
#define	XMS_FREE		0x0A
#define	XMS_MOVE		0x0B
#define	XMS_LOCK		0x0C
#define	XMS_UNLOCK		0x0D
#define	XMS_GETINFO		0x0E
#define	XMS_RESIZE		0x0F

#define	XMS_ALLOCUMB	0x10
#define	XMS_FREEUMB		0x11

//==========================================================================

typedef struct
{
	Sint32	nearheap,farheap,EMSmem,XMSmem,mainmem;
} mminfotype;

//==========================================================================

extern unsigned blockcount;
extern	mminfotype	mminfo;
extern	void* bufferseg;
extern	boolean		mmerror;
//unsigned __SEGS_AVAILABLE__;
Uint32 __PUR_MEM_AVAIL__;
Uint32 __FREE_MEM_AVAIL__;

extern	void		(* beforesort) (void);
extern	void		(* aftersort) (void);

extern char* gp_fartext;

//==========================================================================

void MM_Startup (void);
void MM_Shutdown (void);
void MM_MapEMS (void);

void MM_GetPtr (void** baseptr,Uint32 size);
void MM_FreePtr (void** baseptr);

void MM_SetPurge (void** baseptr, int purge);
void MM_SetLock (void** baseptr, boolean locked);
void MM_SortMem (void);

void MM_ShowMemory (void);

Sint32 MM_UnusedMemory (void);
Sint32 MM_TotalFree (void);
Sint32 MM_LargestAvail (void);

void MM_BombOnError (boolean bomb);

void MML_UseSpace (unsigned segstart, unsigned seglength);

#define LOCKBIT		0x80	// if set in attributes, block cannot be moved

#if IN_DEVELOPMENT
	#define GETNEWBLOCK {if(!mmfree)MML_ClearBlock();mmnew=mmfree;mmfree=mmfree->next;blockcount++;}
#else
	#define GETNEWBLOCK {if(!mmfree)MML_ClearBlock();mmnew=mmfree;mmfree=mmfree->next;}
#endif


typedef struct mmblockstruct
{
	unsigned	start,length;
	unsigned	attributes;
	void** useptr;	// pointer to the segment start
	struct mmblockstruct* next;
} mmblocktype;


#endif
#endif // 0


#define BUFFERSIZE (0x1000) // miscelanious, allways available buffer


extern void* bufferseg;


void MM_Startup();
void MM_Shutdown();


#endif // ID_MM_H
