//===========================================================================
//
//								 LZHUFF COMPRESSION ROUTINES
//										  VERSION 1.0
//
//  				Compression algrythim by Haruhiko OKUMURA
//  						Implementation by Jim T. Row
//
//
//   Copyright (c) 1992 -
//
//===========================================================================
//
// Compiler #ifdef switches
//
// 	LZHUFF_COMPRESSION & LZHUFF_DECOMPRESSION		- not yet functional!
//
// Usage Explanition :
//
//    if LZHUFF_COMPRESSION is defined then the compression code & data is
//    compiled and so-forth for the decompression code.
//
//---------------------------------------------------------------------------


#include <cstdlib>
#include <cstring>

#include "jm_cio.h"
#include "jm_lzh.h"


//===========================================================================
//
//											SWITCHES
//
// NOTE : Make sure the appropriate switches are set in SOFT.c for Softlib
//			 archive support.
//
//===========================================================================


#define INCLUDE_LZH_COMP			1
#define INCLUDE_LZH_DECOMP			1


#define LZH_DYNAMIC_ALLOCATION

#define LZH_ID_MEMORY_ALLOCATION


//===========================================================================
//
//											DEFINES
//
//===========================================================================


#define EXIT_OK 			0
#define EXIT_FAILED 		-1

/* LZSS Parameters */

#define N				4096								/* Size of string buffer */
#define F				30									/* Size of look-ahead buffer */
#define THRESHOLD		2
#define NIL				N									/* End of tree's node  */

/* Huffman coding parameters */

#define N_CHAR  		(256 - THRESHOLD + F)		/* character code (= 0..N_CHAR-1) */
#define T 				(N_CHAR * 2 - 1)				/* Size of table */
#define R 				(T - 1)							/* root position */
#define MAX_FREQ		0x8000                     /* update when cumulative frequency */
																/* reaches to this value */


//==========================================================================
//
//								LOCAL PROTOTYPES
//
//==========================================================================


static void StartHuff(void);
static void reconst(void);
static void update(Sint16 c);


static void DeleteNode(Sint16 p);  /* Deleting node from the tree */
static void InsertNode(Sint16 r);  /* Inserting node to the tree */
static void InitTree(void);  /* Initializing tree */

static void Putcode(
    void*& outfile_ptr,
    Sint16 l,
    Uint16 c);

static void EncodeChar(
    void*& outfile_ptr,
    Uint16 c);

static void EncodePosition(
    void*& outfile_ptr,
    Uint16 c);

static void EncodeEnd(
    void*& outfile_ptr);

static Sint16 GetByte(
    const void*& infile_ptr,
    Uint32* CompressLength);

static Sint16 GetBit(
    const void*& infile_ptr,
    Uint32* CompressLength);

static Sint16 DecodeChar(
    const void*& infile_ptr,
    Uint32* CompressLength);

static Sint16 DecodePosition(
    const void*& infile_ptr,
    Uint32* CompressLength);


//==========================================================================
//
//								USER AVAILABLE VECTORS
//
//==========================================================================




//---------------------------------------------------------------------------
//
// 								LZHUFF DISPLAY VECTORS
//
// These vectors allow you to hook up any form of display you desire for
// displaying the compression/decompression status.
//
// These routines are called inside of the compression/decompression routines
// and pass the orginal size of data and current position within that
// data.  This allows for any kind of "% Done" messages.
//
// Your functions MUST have the following parameters in this order...
//
//   void VectorRoutine(unsigned long OriginalSize,unsigned long CurPosition)
//
//

#if INCLUDE_LZH_COMP
void (*LZH_CompressDisplayVector)(Uint32, Uint32) = NULL;
#endif

#if INCLUDE_LZH_DECOMP
void (*LZH_DecompressDisplayVector)(Uint32, Uint32) = NULL;
#endif




//===========================================================================
//
//											GLOBAL VARIABLES
//
//===========================================================================
	/* pointing children nodes (son[], son[] + 1)*/

Uint16 code, len;
Uint32 textsize = 0, codesize = 0, printcount = 0,datasize;

#ifdef LZH_DYNAMIC_ALLOCATION

Sint16 *son=NULL;

//
// pointing parent nodes.
// area [T..(T + N_CHAR - 1)] are pointers for leaves
//

Sint16 *prnt;
Uint16 *freq;	/* cumulative freq table */
Uint8 *text_buf;

#ifdef LZH_ID_MEMORY_ALLOCATION
Sint16* id_son;
Sint16* id_prnt;
Uint16* id_freq;
Uint8* id_text_buf;
#endif

#else

Sint16 son[T];

	//
	// pointing parent nodes.
	// area [T..(T + N_CHAR - 1)] are pointers for leaves
	//

Sint16 prnt[T + N_CHAR];

Uint16 freq[T + 1];	/* cumulative freq table */

Uint8 text_buf[N + F - 1];

#endif



	//
	// COMPRESSION VARIABLES
	//

#if INCLUDE_LZH_COMP

#ifdef LZH_DYNAMIC_ALLOCATION

static Sint16 *lson, *rson, *dad;

#ifdef LZH_ID_MEMORY_ALLOCATION
Sint16* id_lson;
Sint16* id_rson;
Sint16* id_dad;
#endif
#else

static Sint16 lson[N + 1], rson[N + 257], dad[N + 1];

#endif

static Sint16 match_position,match_length;
Uint16 putbuf = 0;
Uint16 putlen = 0;

	//
	// Tables for encoding/decoding upper 6 bits of
	// sliding dictionary pointer
	//

	//
	// encoder table
	//

Uint8 p_len[64] = {
	0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

Uint8 p_code[64] = {
	0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
	0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9C,
	0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
	0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,
	0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
	0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
	0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};
#endif


	//
	// DECOMPRESSION VARIABLES
	//


	//
	// decoder table
	//

#if INCLUDE_LZH_DECOMP

Uint8 d_code[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
	0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
	0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
	0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
	0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
	0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
	0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
	0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
	0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
	0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
	0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
	0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
	0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

Uint8 d_len[256] = {
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

Uint16 getbuf = 0;
Uint16 getlen = 0;

#endif



//===========================================================================
//
//							COMPRESSION & DECOMPRESSION ROUTINES
//
//===========================================================================


//---------------------------------------------------------------------------
// LZH_Startup
//---------------------------------------------------------------------------
bool LZH_Startup()
{
	if (son)
		return(true);

#ifdef LZH_DYNAMIC_ALLOCATION
#ifdef LZH_ID_MEMORY_ALLOCATION
    id_son = new Sint16[T];
    id_prnt = new Sint16[T + N_CHAR];
    id_freq = new Uint16[T + 1];
    id_text_buf = new Uint8[N + F - 1];
#else
	if (!(son=farmalloc(T*sizeof(*son))))
		return(false);

	if (!(prnt=farmalloc((T+N_CHAR)*sizeof(*prnt))))
		return(false);

	if (!(freq=farmalloc((T+1)*sizeof(*freq))))
		return(false);

	if (!(text_buf=farmalloc((N+F-1)*sizeof(*text_buf))))
		return(false);
#endif

#if INCLUDE_LZH_COMP
#ifdef LZH_ID_MEMORY_ALLOCATION
    id_lson = new Sint16[N + 1];
    id_rson = new Sint16[N + 257];
    id_dad = new Sint16[N + 1];
#else
	if (!(lson=farmalloc((N+1)*sizeof(*lson))))
		return(false);

	if (!(rson=farmalloc((N+257)*sizeof(*rson))))
		return(false);

	if (!(dad=farmalloc((N+1)*sizeof(*dad))))
		return(false);
#endif
#endif
#endif

	return(true);
}

//---------------------------------------------------------------------------
// LZH_Shutdown
//---------------------------------------------------------------------------
void LZH_Shutdown()
{
#ifdef LZH_DYNAMIC_ALLOCATION
#ifdef LZH_ID_MEMORY_ALLOCATION
    delete [] id_son;
    id_son = NULL;

    delete [] id_prnt;
    id_prnt = NULL;

    delete [] id_freq;
    id_freq = NULL;

    delete [] id_text_buf;
    id_text_buf = NULL;
#else
	if (son)
		farfree(son);

	if (prnt)
		farfree(prnt);

	if (freq)
		farfree(freq);

	if (text_buf)
		farfree(text_buf);
#endif

#if INCLUDE_LZH_COMP
#ifdef LZH_ID_MEMORY_ALLOCATION
    delete [] id_lson;
    id_lson = NULL;

    delete [] id_rson;
    id_rson = NULL;

    delete [] id_dad;
    id_dad = NULL;
#else
	if (lson)
		farfree(lson);

	if (rson)
		farfree(rson);

	if (dad)
		farfree(dad);
#endif
#endif

	son=NULL;		// Must be zeroed on shutdown!
#endif
}

//---------------------------------------------------------------------------
//  StartHuff    /* initialize freq tree */
//---------------------------------------------------------------------------
static void StartHuff()
{
	Sint16 i, j;

#ifdef LZH_DYNAMIC_ALLOCATION
#ifdef LZH_ID_MEMORY_ALLOCATION

// Assign _seg pointers to far pointers, always initialized here in case
// the memory manager shifted things around after LZH_Startup() was called.
//
	son=id_son;
	prnt=id_prnt;
	freq=id_freq;
	text_buf=id_text_buf;
	lson=id_lson;
	rson=id_rson;
	dad=id_dad;

#endif
#endif

	for (i = 0; i < N_CHAR; i++) {
		freq[i] = 1;
		son[i] = i + T;
		prnt[i + T] = i;
	}
	i = 0; j = N_CHAR;
	while (j <= R) {
		freq[j] = freq[i] + freq[i + 1];
		son[j] = i;
		prnt[i] = prnt[i + 1] = j;
		i += 2; j++;
	}
	freq[T] = 0xffff;
	prnt[R] = 0;

	printcount=0;

	putbuf=putlen=match_position=match_length=0;
}






//---------------------------------------------------------------------------
//   reconst        /* reconstruct freq tree */
//---------------------------------------------------------------------------
static void reconst()
{
	Sint16 i, j, k;
	Uint16 f, l;

	/* halven cumulative freq for leaf nodes */

	j = 0;

	for (i = 0; i < T; i++)
	{
		if (son[i] >= T)
		{
			freq[j] = (freq[i] + 1) / 2;
			son[j] = son[i];
			j++;
		}
	}

	/* make a tree : first, connect children nodes */

	for (i = 0, j = N_CHAR; j < T; i += 2, j++)
	{
		k = i + 1;
		f = freq[j] = freq[i] + freq[k];

		for (k = j - 1;f < freq[k]; k--);

		k++;
		l = (j - k) * 2;

		memcpy(&freq[k + 1], &freq[k], l);
		freq[k] = f;

		memcpy(&son[k + 1], &son[k], l);
		son[k] = i;
	}

	/* connect parent nodes */

	for (i = 0; i < T; i++)
	{
		if ((k = son[i]) >= T)
		{
			prnt[k] = i;
		}
		else
		{
			prnt[k] = prnt[k + 1] = i;
		}
	}
}






//---------------------------------------------------------------------------
//  update()	 update freq tree
//---------------------------------------------------------------------------
static void update(Sint16 c)
{
	Sint16 i, j, k, l;

	if (freq[R] == MAX_FREQ)
	{
		reconst();
	}

	c = prnt[c + T];

	do {
		k = ++freq[c];

		//
		// swap nodes to keep the tree freq-ordered
		//

		if (k > freq[l = c + 1])
		{
			while (k > freq[++l]);

			l--;
			freq[c] = freq[l];
			freq[l] = k;

			i = son[c];
			prnt[i] = l;
			if (i < T)
				prnt[i + 1] = l;

			j = son[l];
			son[l] = i;

			prnt[j] = c;
			if (j < T)
				prnt[j + 1] = c;

			son[c] = j;

			c = l;
		}
	} while ((c = prnt[c]) != 0);	/* do it until reaching the root */
}




//===========================================================================
//
//									 COMPRESSION ROUTINES
//
//===========================================================================






#if INCLUDE_LZH_COMP


//---------------------------------------------------------------------------
// DeleteNode
//---------------------------------------------------------------------------
static void DeleteNode(Sint16 p)  /* Deleting node from the tree */
{
	Sint16  q;

	if (dad[p] == NIL)
		return;			/* unregistered */

	if (rson[p] == NIL)
		q = lson[p];
	else
	if (lson[p] == NIL)
		q = rson[p];
	else
	{
		q = lson[p];
		if (rson[q] != NIL)
		{
			do {
				q = rson[q];
			} while (rson[q] != NIL);

			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}

		rson[q] = rson[p];
		dad[rson[p]] = q;
	}

	dad[q] = dad[p];

	if (rson[dad[p]] == p)
		rson[dad[p]] = q;
	else
		lson[dad[p]] = q;

	dad[p] = NIL;
}






//---------------------------------------------------------------------------
//  InsertNode
//---------------------------------------------------------------------------
static void InsertNode(Sint16 r)  /* Inserting node to the tree */
{
	Sint16  i, p, cmp;
	Uint8 *key;
	Uint16 c;

	cmp = 1;
	key = &text_buf[r];
	p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;
	match_length = 0;
	for ( ; ; )
	{
		if (cmp >= 0)
		{
			if (rson[p] != NIL)
				p = rson[p];
			else
			{
				rson[p] = r;
				dad[r] = p;
				return;
			}
		}
		else
		{
			if (lson[p] != NIL)
				p = lson[p];
			else
			{
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}


		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)
				break;

		if (i > THRESHOLD)
		{
			if (i > match_length)
			{
				match_position = ((r - p) & (N - 1)) - 1;
				if ((match_length = i) >= F)
					break;
			}

			if (i == match_length)
			{
				if ((c = ((r - p) & (N - 1)) - 1) < match_position)
				{
					match_position = c;
				}
			}
		}
	}

	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];
	dad[lson[p]] = r;
	dad[rson[p]] = r;

	if (rson[dad[p]] == p)
		rson[dad[p]] = r;
	else
		lson[dad[p]] = r;

	dad[p] = NIL;  /* remove p */
}





//---------------------------------------------------------------------------
// InitTree
//---------------------------------------------------------------------------
static void InitTree(void)  /* Initializing tree */
{
	Sint16  i;

	for (i = N + 1; i <= N + 256; i++)
		rson[i] = NIL;			/* root */

	for (i = 0; i < N; i++)
		dad[i] = NIL;			/* node */
}






//---------------------------------------------------------------------------
//  Putcode
//---------------------------------------------------------------------------
// output c bits
static void Putcode(
    void*& outfile_ptr,
    Sint16 l,
    Uint16 c)
{
    putbuf |= c >> putlen;

    putlen += l;

    if (putlen >= 8) {
        ::CIO_WritePtr(outfile_ptr, putbuf >> 8);
        ++codesize;

        putlen -= 8;
        if (putlen >= 8) {
            ::CIO_WritePtr(outfile_ptr, static_cast<Uint8>(putbuf));
            ++codesize;

            putlen -= 8;
            putbuf = c << (l - putlen);
        } else
            putbuf <<= 8;
    }
}


//---------------------------------------------------------------------------
//  EncodeChar
//---------------------------------------------------------------------------
static void EncodeChar(
    void*& outfile_ptr,
    Uint16 c)
{
    Uint16 i;
    Sint16 j, k;

    i = 0;
    j = 0;
    k = prnt[c + T];

    /// search connections from leaf node to the root

    do {
        i >>= 1;

        //
        // if node's address is odd, output 1 else output 0
        //

        if ((k & 1) != 0)
            i += 0x8000;

        ++j;
        k = prnt[k];
    } while (k != R);

    ::Putcode(outfile_ptr, j, i);

    code = i;
    len = j;
    ::update(c);
}


//---------------------------------------------------------------------------
// EncodePosition
//---------------------------------------------------------------------------
static void EncodePosition(
    void*& outfile_ptr,
    Uint16 c)
{
    Uint16 i;

    //
    // output upper 6 bits with encoding
    //

    i = c >> 6;
    ::Putcode(outfile_ptr, p_len[i], static_cast<Uint16>(p_code[i]) << 8);

    //
    // output lower 6 bits directly
    //

    ::Putcode(outfile_ptr, 6, (c & 0x3F) << 10);
}


//---------------------------------------------------------------------------
// EncodeEnd
//---------------------------------------------------------------------------
static void EncodeEnd(
    void*& outfile_ptr)
{
    if (putlen != 0) {
        ::CIO_WritePtr(outfile_ptr, putbuf >> 8);
        ++codesize;
    }
}

#endif





//===========================================================================
//
//									DECOMPRESSION ROUTINES
//
//===========================================================================



#if INCLUDE_LZH_DECOMP

//---------------------------------------------------------------------------
// GetByte
//---------------------------------------------------------------------------
static Sint16 GetByte(
    const void*& infile_ptr,
    Uint32* CompressLength)
{
    Uint16 i;

    while (getlen <= 8) {
        if (*CompressLength) {
            i = ::CIO_ReadPtr(infile_ptr);
            (*CompressLength)--;
        } else
            i = 0;

        getbuf |= i << (8 - getlen);
        getlen += 8;
    }

    i = getbuf;
    getbuf <<= 8;
    getlen -= 8;
    return i >> 8;
}






//---------------------------------------------------------------------------
// GetBit
//---------------------------------------------------------------------------
static Sint16 GetBit(
    const void*& infile_ptr,
    Uint32* CompressLength)
{
    Sint16 i;

    while (getlen <= 8) {
        if (*CompressLength) {
            i = ::CIO_ReadPtr(infile_ptr);
            (*CompressLength)--;
        } else
            i = 0;

        getbuf |= i << (8 - getlen);
        getlen += 8;
    }

    i = getbuf;
    getbuf <<= 1;
    --getlen;
    return i < 0;
}



//---------------------------------------------------------------------------
// DecodeChar
//---------------------------------------------------------------------------
static Sint16 DecodeChar(
    const void*& infile_ptr,
    Uint32* CompressLength)
{
    Uint16 c;

    c = son[R];

    /*
    * start searching tree from the root to leaves.
    * choose node #(son[]) if input bit == 0
    * else choose #(son[]+1) (input bit == 1)
    */

    while (c < T) {
        c += ::GetBit(infile_ptr, CompressLength);
        c = son[c];
    }

    c -= T;
    ::update(c);
    return c;
}




//---------------------------------------------------------------------------
// DecodePosition
//---------------------------------------------------------------------------
static Sint16 DecodePosition(
    const void*& infile_ptr,
    Uint32* CompressLength)
{
    Uint16 i;
    Uint16 j;
    Uint16 c;

    //
    // decode upper 6 bits from given table
    //

    i = ::GetByte(infile_ptr, CompressLength);
    c = static_cast<Uint16>(d_code[i]) << 6;
    j = d_len[i];

    //
    // input lower 6 bits directly
    //

    j -= 2;
    while (j--)
        i = (i << 1) + ::GetBit(infile_ptr, CompressLength);

    return c | (i & 0x3F);
}

#endif





//===========================================================================
//
//									EXTERNAL REFERENCED
//							  COMPRESSION & DECOMPRESSION
//									     ROUTINES
//
//===========================================================================




#if INCLUDE_LZH_DECOMP

//---------------------------------------------------------------------------
// LZH_Decompress()
//---------------------------------------------------------------------------
Sint32 LZH_Decompress(
    const void* infile,
    void* outfile,
    Uint32 OriginalLength,
    Uint32 CompressLength)
{
	Sint16  i, j, k, r, c;
	Sint32 count;

	datasize = textsize = OriginalLength;
	getbuf = 0;
	getlen = 0;

	if (textsize == 0)
		return(0);

	StartHuff();
	for (i = 0; i < N - F; i++)
		text_buf[i] = ' ';

	r = N - F;

	for (count = 0; count < static_cast<Sint32>(textsize); )
	{
        c = ::DecodeChar(infile, &CompressLength);

		if (c < 256)
		{
            ::CIO_WritePtr(outfile, static_cast<Uint8>(c));

			datasize--;								// Dec # of bytes to write

			text_buf[r++] = static_cast<Uint8>(c);
			r &= (N - 1);
			count++;									// inc count of bytes written
		}
		else
		{
            i = (r - ::DecodePosition(infile, &CompressLength) - 1) & (N - 1);

			j = c - 255 + THRESHOLD;

			for (k = 0; k < j; k++)
			{
				c = text_buf[(i + k) & (N - 1)];

                ::CIO_WritePtr(outfile, static_cast<Uint8>(c));

				datasize--;							// dec count of bytes to write

				text_buf[r++] = static_cast<Uint8>(c);
				r &= (N - 1);
				count++;								// inc count of bytes written
			}
		}

		if (LZH_DecompressDisplayVector && (count > static_cast<Sint32>(printcount)))
		{
			LZH_DecompressDisplayVector(OriginalLength,OriginalLength-datasize);
			printcount += 1024;
		}
	}

	if (LZH_DecompressDisplayVector)
		LZH_DecompressDisplayVector(OriginalLength,OriginalLength);

	return(count);
}

#endif





#if INCLUDE_LZH_COMP

//---------------------------------------------------------------------------
// LZH_Compress()
//---------------------------------------------------------------------------
int LZH_Compress(
    const void* infile,
    void* outfile,
    Uint32 DataLength)
{
	Sint16  i, c, len, r, s, last_match_length;

	textsize = DataLength;

	if (textsize == 0)
		return(0);

	getbuf = 0;
	getlen = 0;
	textsize = 0;			/* rewind and rescan */
	codesize = 0;
	datasize = 0;			// Init our counter of ReadData...
	StartHuff();
	InitTree();

	s = 0;
	r = N - F;

	for (i = s; i < r; i++)
		text_buf[i] = ' ';

	for (len = 0; len < F && (DataLength > datasize); len++)
	{
        c = ::CIO_ReadPtr(infile);

		datasize++;							// Dec num of bytes to compress
		text_buf[r + len] = static_cast<Uint8>(c);
	}

	textsize = len;

	for (i = 1; i <= F; i++)
		InsertNode(r - i);

	InsertNode(r);

	do {
		if (match_length > len)
			match_length = len;

		if (match_length <= THRESHOLD)
		{
			match_length = 1;
            ::EncodeChar(outfile, text_buf[r]);
		}
		else
		{
            ::EncodeChar(outfile, 255 - THRESHOLD + match_length);
            ::EncodePosition(outfile, match_position);
		}

		last_match_length = match_length;

		for (i = 0; i < last_match_length && (DataLength > datasize); i++)
		{
            c = ::CIO_ReadPtr(infile);

			datasize++;

			DeleteNode(s);
			text_buf[s] = static_cast<Uint8>(c);

			if (s < F - 1)
				text_buf[s + N] = static_cast<Uint8>(c);

			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			InsertNode(r);
		}

		if (LZH_CompressDisplayVector && ((textsize += i) > printcount))
		{
			LZH_CompressDisplayVector(DataLength,datasize);
			printcount += 1024;
		}


		while (i++ < last_match_length)
		{
			DeleteNode(s);
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			if (--len)
				InsertNode(r);
		}

	} while (len > 0);

    ::EncodeEnd(outfile);

	if (LZH_CompressDisplayVector)
		LZH_CompressDisplayVector(DataLength,DataLength);

	return(codesize);
}


#endif

