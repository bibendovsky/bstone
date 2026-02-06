/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Saved game stuff.

#include "bstone_saved_game.h"

#include <algorithm>
#include <iterator>

#include "bstone_ascii.h"
#include "bstone_char_conv.h"
#include "bstone_endian.h"
#include "bstone_exception.h"

namespace bstone {

void SgChunkHeader::serialize(Stream& stream) const
try {
	unsigned char buffer[sg_chunk_header_size];
	endian::write_u32_le(id, buffer);
	endian::write_s32_le(size, buffer + 4);
	stream.write_exactly(buffer, sg_chunk_header_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SgChunkHeader::deserialize(Stream& stream)
try {
	unsigned char buffer[sg_chunk_header_size];
	stream.read_exactly(buffer, sg_chunk_header_size);
	id = endian::read_u32_le(buffer);
	size = endian::read_s32_le(buffer + 4);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

FourCc sg_make_numbered_four_cc(char ch_0, char ch_1, int number)
try {
	if (number < 0 || number > 0xFF)
	{
		BSTONE_THROW_STATIC_SOURCE("Number out of range.");
	}

	const auto number_as_byte = static_cast<std::uint8_t>(number);
	char number_chars[2] = {};
	const auto number_chars_begin = std::begin(number_chars);
	const auto number_chars_end = std::end(number_chars);
	bytes_to_hex_chars(&number_as_byte, &number_as_byte + 1, number_chars_begin, number_chars_end);
	ascii::to_upper(number_chars_begin, number_chars_end);

	return FourCc{ch_0, ch_1, number_chars[0], number_chars[1]};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

FourCc sg_make_level_four_cc(int number)
{
	return sg_make_numbered_four_cc('L', 'V', number);
}

FourCc sg_make_overlay_four_cc(int number)
{
	return sg_make_numbered_four_cc('O', 'V', number);
}

int sg_find_chunk(bstone::FourCc four_cc, bstone::Stream& stream)
{
	stream.set_position(0);

	const auto stream_size = stream.get_size();
	auto stream_position = std::int64_t{};

	while (stream_position != stream_size)
	{
		auto chunk_header = bstone::SgChunkHeader{};
		chunk_header.deserialize(stream);

		if (chunk_header.id == four_cc.get_value())
		{
			return chunk_header.size;
		}

		if (chunk_header.size < 0)
		{
			break;
		}

		stream.skip(chunk_header.size);
		stream_position += sg_chunk_header_size;
		stream_position += chunk_header.size;
	}

	stream.seek(0, bstone::StreamOrigin::end);

	return 0;
}

void sg_delete_chunk(bstone::FourCc four_cc, bstone::MemoryStream& stream)
{
	stream.set_position(0);
	const auto chunk_size = sg_find_chunk(four_cc, stream);

	if (chunk_size <= 0)
	{
		return;
	}

	const auto chunk_offset = stream.get_position() - 8;
	const auto total_chunk_size = chunk_size + 8;
	auto data = stream.get_data();
	const auto data_size = stream.get_size();

	std::copy_n(
		data + chunk_offset + total_chunk_size,
		data_size - chunk_offset - total_chunk_size,
		data + chunk_offset);

	stream.set_size(data_size - total_chunk_size);
}

} // namespace bstone
