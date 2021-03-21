/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include "id_pm.h"

#include "id_ca.h"
#include "id_heads.h"
#include "bstone_endian.h"
#include "bstone_memory_binary_reader.h"


// File specific variables
int ChunksInFile = 0;
int PMSpriteStart = 0;
int PMSoundStart = 0;


namespace
{


using RawData = std::vector<std::uint8_t>;

bstone::FileStream PageFile;
RawData raw_data;
std::uint32_t* chunks_offsets = nullptr;


} // namespace


void open_page_file()
{
	ca_open_resource(AssetsResourceType::vswap, PageFile);

	const auto file_length = PageFile.get_size();

	if (file_length > 4 * 1024 * 1024)
	{
		Quit("Page file is too large.");
	}

	const auto file_length_32 = static_cast<std::int32_t>(file_length);

	raw_data.resize(file_length_32 + PMPageSize);

	if (PageFile.read(raw_data.data(), file_length_32) != file_length_32)
	{
		Quit("Page file read error.");
	}

	bstone::MemoryBinaryReader reader(raw_data.data(), file_length);

	ChunksInFile = bstone::Endian::little(reader.read_u16());
	PMSpriteStart = bstone::Endian::little(reader.read_u16());
	PMSoundStart = bstone::Endian::little(reader.read_u16());

	chunks_offsets = reinterpret_cast<std::uint32_t*>(&raw_data[6]);

	for (auto i = 0; i < (ChunksInFile + 1); ++i)
	{
		bstone::Endian::little(chunks_offsets[i]);
	}
}

void PM_Startup()
{
	PM_Shutdown();
	open_page_file();
}

void PM_Shutdown()
{
	PageFile.close();

	ChunksInFile = 0;
	PMSpriteStart = 0;
	PMSoundStart = 0;

	RawData{}.swap(raw_data);

	chunks_offsets = nullptr;
}

void* PM_GetPage(
	int page_number)
{
	if (page_number < 0 || page_number >= ChunksInFile)
	{
		Quit("Page number out of range.");
	}

	const auto offset = chunks_offsets[page_number];

	if (offset == 0)
	{
		Quit("Tried to load a sparse page.");
	}

	return &raw_data[offset];
}

void* PM_GetSoundPage(
	int page_number)
{
	return PM_GetPage(PMSoundStart + page_number);
}

void* PM_GetSpritePage(
	int page_number)
{
	return PM_GetPage(PMSpriteStart + page_number);
}
