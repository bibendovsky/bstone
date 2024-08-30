/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cstring>

#include <memory>

#include "id_heads.h"
#include "id_ca.h"
#include "jm_cio.h"
#include "jm_io.h"
#include "jm_lzh.h"


int IO_LoadFile(
	const char* filename,
	void** dst)
{
	char buffer[5] = {0, 0, 0, 0, 0};
	bstone::FileStream handle;
	std::int32_t size = 0;

	handle.open(filename);

	if (!handle.is_open())
	{
		return size;
	}

	handle.read(buffer, 4);

	if (!strcmp(buffer, "JAMP"))
	{
		struct JAMPHeader head;

		handle.read(&head, sizeof(head));
		size = head.OriginalLen;
		switch (head.CompType)
		{
		case ct_LZH:
			LZH_Startup();

			*dst = new char[head.OriginalLen];

			{
				std::unique_ptr<std::uint8_t> compressed_buffer(
					new std::uint8_t[head.CompressLen]);

				LZH_Decompress(compressed_buffer.get(), *dst,
					size, head.CompressLen);
			}

			LZH_Shutdown();
			break;

		case ct_LZW:
			Quit("No code for LZW compression.");

		default:
			Quit("Unknown compression type.");
		}
	}
	else
	{
		handle.set_position(0);
		size = static_cast<std::int32_t>(handle.get_size());
		*dst = new char[size];
		if (handle.read(*dst, size) != size)
		{
			return size;
		}
	}

	return size;
}
