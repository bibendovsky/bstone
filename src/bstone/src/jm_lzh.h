/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_JM_LZH_INCLUDED
#define BSTONE_JM_LZH_INCLUDED


#include <cstdint>


int LZH_Compress(
	const std::uint8_t* in_buffer,
	std::uint8_t* out_buffer,
	int in_length);

int LZH_Decompress(
	const std::uint8_t* in_buffer,
	std::uint8_t* out_buffer,
	int uncompressed_length,
	int compressed_length);


#endif // BSTONE_JM_LZH_INCLUDED
