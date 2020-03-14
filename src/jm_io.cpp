/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
