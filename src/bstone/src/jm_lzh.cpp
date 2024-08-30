/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


// ===========================================================================
//
// LZHUFF COMPRESSION ROUTINES
// VERSION 1.0
//
// Compression algrythim by Haruhiko OKUMURA
// Implementation by Jim T. Row
//
//
// Copyright (c) 1992 -
//
// ===========================================================================
//
// Compiler #ifdef switches
//
// LZHUFF_COMPRESSION & LZHUFF_DECOMPRESSION - not yet functional!
//
// Usage Explanition :
//
//    if LZHUFF_COMPRESSION is defined then the compression code & data is
//    compiled and so-forth for the decompression code.
//
// ---------------------------------------------------------------------------


#include <algorithm>
#include <array>
#include <memory>
#include <string>

#include "jm_lzh.h"

#include "bstone_exception.h"


namespace
{


// LZSS Parameters

constexpr auto N = static_cast<std::int16_t>(4096); // Size of string buffer
constexpr auto F = static_cast<std::int16_t>(30); // Size of look-ahead buffer
constexpr auto THRESHOLD = static_cast<std::int16_t>(2);
constexpr auto NIL = static_cast<std::int16_t>(N); // End of tree's node

// Huffman coding parameters

constexpr auto N_CHAR = static_cast<std::int16_t>(256 - THRESHOLD + F); // character code (= 0..N_CHAR-1)
constexpr auto T = static_cast<std::int16_t>((N_CHAR * 2) - 1); // Size of table
constexpr auto R = static_cast<std::int16_t>(T - 1); // root position

// update when cumulative frequency
// reaches to this value
constexpr auto MAX_FREQ = static_cast<std::uint16_t>(0x8000);


void StartHuff();

void reconst();

void update(
	std::int16_t c);

// Deleting node from the tree
void DeleteNode(
	std::int16_t p);

// Inserting node to the tree
void InsertNode(
	std::int16_t r);

// Initializing tree
void InitTree();

void Putcode(
	std::uint8_t*& buffer,
	std::int16_t l,
	std::uint16_t c);

void EncodeChar(
	std::uint8_t*& buffer,
	std::uint16_t c);

void EncodePosition(
	std::uint8_t*& buffer,
	std::uint16_t c);

void EncodeEnd(
	std::uint8_t*& buffer);

std::int16_t GetByte(
	const std::uint8_t*& buffer,
	int& length);

std::int16_t GetBit(
	const std::uint8_t*& buffer,
	int& length);

std::int16_t DecodeChar(
	const std::uint8_t*& buffer,
	int& length);

std::int16_t DecodePosition(
	const std::uint8_t*& buffer,
	int& length);


// ===========================================================================
//
// GLOBAL VARIABLES
//
// ===========================================================================

// pointing children nodes (son[], son[] + 1)

std::uint16_t code;
std::uint16_t len;

int textsize;
int codesize;
int datasize;

std::array<std::int16_t, T> son;

//
// pointing parent nodes.
// area [T..(T + N_CHAR - 1)] are pointers for leaves
//

std::array<std::int16_t, T + N_CHAR> prnt;

std::array<std::uint16_t, T + 1> freq; // cumulative freq table

std::array<std::uint8_t, N + F - 1> text_buf;


//
// COMPRESSION VARIABLES
//

std::array<std::int16_t, N + 1> lson;
std::array<std::int16_t, N + 257> rson;
std::array<std::int16_t, N + 1> dad;

std::int16_t match_position;
std::int16_t match_length;

std::uint16_t putbuf;
std::uint16_t putlen;

//
// Tables for encoding/decoding upper 6 bits of
// sliding dictionary pointer
//

//
// encoder table
//

const std::array<std::uint8_t, 64> p_len =
{
	0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

const std::array<std::uint8_t, 64> p_code =
{
	0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
	0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9C,
	0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
	0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,
	0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
	0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
	0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};


//
// DECOMPRESSION VARIABLES
//


//
// decoder table
//

const std::array<std::uint8_t, 256> d_code =
{
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

const std::array<std::uint8_t, 256> d_len =
{
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

std::uint16_t getbuf;
std::uint16_t getlen;


// ===========================================================================
//
// COMPRESSION & DECOMPRESSION ROUTINES
//
// ===========================================================================

// initialize freq tree
void StartHuff()
{
	for (std::int16_t i = 0; i < N_CHAR; ++i)
	{
		freq[i] = 1;
		son[i] = i + T;
		prnt[i + T] = i;
	}

	for (std::int16_t i = 0, j = N_CHAR; j <= R; i += 2, ++j)
	{
		freq[j] = freq[i] + freq[i + 1];
		son[j] = i;
		prnt[i] = prnt[i + 1] = j;
	}

	freq[T] = 0xFFFF;
	prnt[R] = 0;

	putbuf = 0;
	putlen = 0;
	match_position = 0;
	match_length = 0;
}

// reconstruct freq tree
void reconst()
{
	// halven cumulative freq for leaf nodes

	{
		std::int16_t j = 0;

		for (std::int16_t i = 0; i < T; ++i)
		{
			if (son[i] >= T)
			{
				freq[j] = (freq[i] + 1) / 2;
				son[j] = son[i];
				++j;
			}
		}
	}

	// make a tree : first, connect children nodes

	for (std::int16_t i = 0, j = N_CHAR; j < T; i += 2, ++j)
	{
		std::int16_t k = i + 1;
		std::uint16_t f = freq[i] + freq[k];

		freq[j] = f;

		for (k = j - 1; f < freq[k]; --k)
		{
		}

		++k;

		std::uint16_t l = (j - k) * 2;

		std::copy_n(freq.cbegin() + k, l, freq.begin() + k + 1);

		freq[k] = f;

		std::copy_n(son.cbegin() + k, l, son.begin() + k + 1);

		son[k] = i;
	}

	// connect parent nodes

	for (std::int16_t i = 0; i < T; ++i)
	{
		std::int16_t k = son[i];

		if (k >= T)
		{
			prnt[k] = i;
		}
		else
		{
			prnt[k] = i;
			prnt[k + 1] = i;
		}
	}
}

// update freq tree
void update(
	std::int16_t c)
{
	if (freq[R] == MAX_FREQ)
	{
		reconst();
	}

	c = prnt[c + T];

	do
	{
		const std::int16_t k = ++freq[c];

		//
		// swap nodes to keep the tree freq-ordered
		//

		std::int16_t l = c + 1;

		if (k > freq[l])
		{
			while (k > freq[++l])
			{
			}

			--l;
			freq[c] = freq[l];
			freq[l] = k;

			const std::int16_t i = son[c];

			prnt[i] = l;

			if (i < T)
			{
				prnt[i + 1] = l;
			}

			const std::int16_t j = son[l];

			son[l] = i;

			prnt[j] = c;

			if (j < T)
			{
				prnt[j + 1] = c;
			}

			son[c] = j;

			c = l;
		}

		c = prnt[c];
	} while (c != 0); // do it until reaching the root
}


// ===========================================================================
//
// COMPRESSION ROUTINES
//
// ===========================================================================


// Deleting node from the tree
void DeleteNode(
	std::int16_t p)
{
	if (dad[p] == NIL)
	{
		return; // unregistered
	}

	std::int16_t q;

	if (rson[p] == NIL)
	{
		q = lson[p];
	}
	else if (lson[p] == NIL)
	{
		q = rson[p];
	}
	else
	{
		q = lson[p];

		if (rson[q] != NIL)
		{
			do
			{
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
	{
		rson[dad[p]] = q;
	}
	else
	{
		lson[dad[p]] = q;
	}

	dad[p] = NIL;
}

// Inserting node to the tree
void InsertNode(
	std::int16_t r)
{
	std::int16_t cmp = 1;
	auto key = &text_buf[r];
	std::int16_t p = N + 1 + key[0];

	rson[r] = NIL;
	lson[r] = NIL;
	match_length = 0;

	for ( ; ; )
	{
		if (cmp >= 0)
		{
			if (rson[p] != NIL)
			{
				p = rson[p];
			}
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
			{
				p = lson[p];
			}
			else
			{
				lson[p] = r;
				dad[r] = p;

				return;
			}
		}

		std::int16_t i;

		for (i = 1; i < F; ++i)
		{
			cmp = key[i] - text_buf[p + i];

			if (cmp != 0)
			{
				break;
			}
		}

		if (i > THRESHOLD)
		{
			if (i > match_length)
			{
				match_position = ((r - p) & (N - 1)) - 1;
				match_length = i;

				if (match_length >= F)
				{
					break;
				}
			}

			if (i == match_length)
			{
				std::uint16_t c = ((r - p) & (N - 1)) - 1;

				if (c < match_position)
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
	{
		rson[dad[p]] = r;
	}
	else
	{
		lson[dad[p]] = r;
	}

	dad[p] = NIL; // remove p
}

// Initializing tree
void InitTree()
{
	for (std::int16_t i = N + 1; i <= N + 256; ++i)
	{
		rson[i] = NIL; // root

	}

	for (std::int16_t i = 0; i < N; ++i)
	{
		dad[i] = NIL; // node
	}
}

// output c bits
void Putcode(
	std::uint8_t*& buffer,
	std::int16_t l,
	std::uint16_t c)
{
	putbuf |= c >> putlen;

	putlen += l;

	if (putlen >= 8)
	{
		*buffer++ = putbuf >> 8;
		++codesize;

		putlen -= 8;

		if (putlen >= 8)
		{
			*buffer++ = static_cast<std::uint8_t>(putbuf);
			++codesize;

			putlen -= 8;
			putbuf = c << (l - putlen);
		}
		else
		{
			putbuf <<= 8;
		}
	}
}

void EncodeChar(
	std::uint8_t*& buffer,
	std::uint16_t c)
{
	std::uint16_t i = 0;
	std::int16_t j = 0;
	std::int16_t k = prnt[c + T];

	// search connections from leaf node to the root

	do
	{
		i >>= 1;

		//
		// if node's address is odd, output 1 else output 0
		//

		if ((k & 1) != 0)
		{
			i += 0x8000;
		}

		++j;
		k = prnt[k];
	} while (k != R);

	Putcode(buffer, j, i);

	code = i;
	len = j;

	update(c);
}

void EncodePosition(
	std::uint8_t*& buffer,
	std::uint16_t c)
{
	//
	// output upper 6 bits with encoding
	//

	const std::uint16_t i = c >> 6;
	Putcode(buffer, p_len[i], static_cast<std::uint16_t>(p_code[i]) << 8);

	//
	// output lower 6 bits directly
	//

	Putcode(buffer, 6, (c & 0x3F) << 10);
}

void EncodeEnd(
	std::uint8_t*& buffer)
{
	if (putlen != 0)
	{
		*buffer++ = putbuf >> 8;
		++codesize;
	}
}


// ===========================================================================
//
// DECOMPRESSION ROUTINES
//
// ===========================================================================


// ---------------------------------------------------------------------------
// GetByte
// ---------------------------------------------------------------------------
std::int16_t GetByte(
	const std::uint8_t*& buffer,
	int& length)
{
	std::uint16_t i;

	while (getlen <= 8)
	{
		if (length != 0)
		{
			i = *buffer++;
			--length;
		}
		else
		{
			i = 0;
		}

		getbuf |= i << (8 - getlen);
		getlen += 8;
	}

	i = getbuf;
	getbuf <<= 8;
	getlen -= 8;

	return i >> 8;
}

std::int16_t GetBit(
	const std::uint8_t*& buffer,
	int& length)
{
	std::int16_t i;

	while (getlen <= 8)
	{
		if (length != 0)
		{
			i = *buffer++;
			--length;
		}
		else
		{
			i = 0;
		}

		getbuf |= i << (8 - getlen);
		getlen += 8;
	}

	i = getbuf;
	getbuf <<= 1;
	--getlen;

	return i < 0;
}

std::int16_t DecodeChar(
	const std::uint8_t*& buffer,
	int& length)
{
	std::uint16_t c = son[R];

	//
	// start searching tree from the root to leaves.
	// choose node #(son[]) if input bit == 0
	// else choose #(son[]+1) (input bit == 1)
	//

	while (c < T)
	{
		c += GetBit(buffer, length);
		c = son[c];
	}

	c -= T;
	update(c);

	return c;
}

std::int16_t DecodePosition(
	const std::uint8_t*& buffer,
	int& length)
{
	//
	// decode upper 6 bits from given table
	//

	std::uint16_t i = GetByte(buffer, length);
	const std::uint16_t c = static_cast<std::uint16_t>(d_code[i]) << 6;
	std::uint16_t j = d_len[i];

	//
	// input lower 6 bits directly
	//

	j -= 2;

	while (j-- != 0)
	{
		i = (i << 1) + GetBit(buffer, length);
	}

	return c | (i & 0x3F);
}


} // namespace


// ===========================================================================
//
// EXTERNAL REFERENCED
// COMPRESSION & DECOMPRESSION
// ROUTINES
//
// ===========================================================================

int LZH_Decompress(
	const std::uint8_t* in_buffer,
	std::uint8_t* out_buffer,
	int uncompressed_length,
	int compress_length)
try {
	if (uncompressed_length < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Uncompressed length out of range.");
	}

	if (compress_length < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Compressed length out of range.");
	}

	if (uncompressed_length == 0 || compress_length == 0)
	{
		return 0;
	}

	if (in_buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null input buffer.");
	}

	if (out_buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null output buffer.");
	}

	datasize = uncompressed_length;
	textsize = uncompressed_length;
	getbuf = 0;
	getlen = 0;

	StartHuff();

	for (std::int16_t i = 0; i < N - F; ++i)
	{
		text_buf[i] = ' ';
	}

	std::int16_t r = N - F;

	int count = 0;

	while (count < textsize)
	{
		std::int16_t c = DecodeChar(in_buffer, compress_length);

		if (c < 256)
		{
			*out_buffer++ = static_cast<std::uint8_t>(c);

			datasize--; // Dec # of bytes to write

			text_buf[r++] = static_cast<std::uint8_t>(c);
			r &= (N - 1);
			++count; // inc count of bytes written
		}
		else
		{
			const std::int16_t i = (r - DecodePosition(in_buffer, compress_length) - 1) & (N - 1);

			const std::int16_t j = c - 255 + THRESHOLD;

			for (std::int16_t k = 0; k < j; ++k)
			{
				c = text_buf[(i + k) & (N - 1)];

				*out_buffer++ = static_cast<std::uint8_t>(c);

				--datasize; // dec count of bytes to write

				text_buf[r++] = static_cast<std::uint8_t>(c);
				r &= (N - 1);
				++count; // inc count of bytes written
			}
		}
	}

	return count;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int LZH_Compress(
	const std::uint8_t* in_buffer,
	std::uint8_t* out_buffer,
	int in_length)
try {
	if (in_length < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Data length out of range.");
	}

	if (in_length == 0)
	{
		return 0;
	}

	if (in_buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null input buffer.");
	}

	if (out_buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null output buffer.");
	}

	getbuf = 0;
	getlen = 0;
	textsize = 0; // rewind and rescan
	codesize = 0;
	datasize = 0; // Init our counter of ReadData...

	StartHuff();
	InitTree();

	std::int16_t s = 0;
	std::int16_t r = N - F;

	for (std::int16_t i = s; i < r; ++i)
	{
		text_buf[i] = ' ';
	}

	std::int16_t length;

	for (length = 0; length < F && (in_length > datasize); ++length)
	{
		std::int16_t c = *in_buffer++;

		datasize++; // Dec num of bytes to compress
		text_buf[r + length] = static_cast<std::uint8_t>(c);
	}

	textsize = length;

	for (std::int16_t i = 1; i <= F; ++i)
	{
		InsertNode(r - i);
	}

	InsertNode(r);

	do
	{
		if (match_length > length)
		{
			match_length = length;
		}

		if (match_length <= THRESHOLD)
		{
			match_length = 1;
			EncodeChar(out_buffer, text_buf[r]);
		}
		else
		{
			EncodeChar(out_buffer, 255 - THRESHOLD + match_length);
			EncodePosition(out_buffer, match_position);
		}

		const std::int16_t last_match_length = match_length;

		std::int16_t i;

		for (i = 0; i < last_match_length && (in_length > datasize); ++i)
		{
			const std::int16_t c = *in_buffer++;

			++datasize;

			DeleteNode(s);
			text_buf[s] = static_cast<std::uint8_t>(c);

			if (s < F - 1)
			{
				text_buf[s + N] = static_cast<std::uint8_t>(c);
			}

			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);

			InsertNode(r);
		}

		while (i++ < last_match_length)
		{
			DeleteNode(s);

			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);

			if (--length != 0)
			{
				InsertNode(r);
			}
		}
	} while (length > 0);

	EncodeEnd(out_buffer);

	return codesize;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED
