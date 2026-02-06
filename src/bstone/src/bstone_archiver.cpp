/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_archiver.h"
#include "bstone_assert.h"
#include "bstone_crc32.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include <cstddef>
#include <algorithm>

namespace bstone {

void Archiver::open(Stream& stream)
{
	close();
	stream_ = &stream;
}

void Archiver::close()
{
	crc32_.reset();
	stream_ = nullptr;
}

bool Archiver::is_open() const
{
	return stream_ != nullptr;
}

bool Archiver::read_bool()
{
	return read_uint8() != 0;
}

std::int8_t Archiver::read_int8()
{
	return static_cast<std::int8_t>(read_uint8());
}

std::uint8_t Archiver::read_uint8()
try {
	BSTONE_ASSERT(is_open());
	constexpr int value_size = 1;
	std::uint8_t value;
	stream_->read_exactly(&value, value_size);
	crc32_.update(&value, value_size);
	return value;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::int16_t Archiver::read_int16()
{
	return static_cast<std::int16_t>(read_uint16());
}

std::uint16_t Archiver::read_uint16()
try {
	BSTONE_ASSERT(is_open());
	constexpr int value_size = 2;
	std::byte bytes[value_size];
	stream_->read_exactly(bytes, value_size);
	crc32_.update(bytes, value_size);
	return endian::read_u16_le(bytes);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::int32_t Archiver::read_int32()
{
	return static_cast<std::int32_t>(read_uint32());
}

std::uint32_t Archiver::read_uint32()
try {
	BSTONE_ASSERT(is_open());
	constexpr int value_size = 4;
	std::byte bytes[value_size];
	stream_->read_exactly(bytes, value_size);
	crc32_.update(bytes, value_size);
	return endian::read_u32_le(bytes);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::read_char_array(char* items, int item_count)
try {
	read_uint8_array(reinterpret_cast<std::uint8_t*>(items), item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::read_int8_array(std::int8_t* items, int item_count)
{
	read_uint8_array(reinterpret_cast<std::uint8_t*>(items), item_count);
}

void Archiver::read_uint8_array(std::uint8_t* items, int item_count)
try {
	BSTONE_ASSERT(is_open());
	BSTONE_ASSERT(items != nullptr);
	BSTONE_ASSERT(item_count > 0);
	stream_->read_exactly(items, item_count);
	crc32_.update(items, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::read_uint16_array(std::uint16_t* items, int item_count)
try {
	constexpr int item_size = 2;
	const int items_size = item_count * item_size;
	constexpr int max_buffer_size = 128;
	constexpr int items_per_buffer = max_buffer_size / item_size;
	std::byte buffer[max_buffer_size];
	int item_offset = 0;
	while (item_count > 0)
	{
		const int to_read_count = std::min(item_count, items_per_buffer);
		const int to_read_size = to_read_count * item_size;
		stream_->read_exactly(buffer, to_read_size);
		crc32_.update(buffer, to_read_size);
		for (int i = 0; i < to_read_count; ++i)
		{
			items[item_offset + i] = endian::read_u16_le(buffer + i * item_size);
		}
		item_count -= to_read_count;
		item_offset += to_read_count;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int Archiver::read_string(int max_string_length, char* string)
try {
	BSTONE_ASSERT(max_string_length >= 0 && max_string_length <= INT32_MAX);
	BSTONE_ASSERT(string != nullptr);
	const std::int32_t archived_string_length = read_int32();
	if (archived_string_length < 0 || archived_string_length > max_string_length)
	{
		BSTONE_THROW_STATIC_SOURCE("Archived string length out of range.");
	}
	stream_->read_exactly(string, archived_string_length);
	return archived_string_length;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::read_checksum()
try {
	BSTONE_ASSERT(is_open());
	constexpr int value_size = 4;
	std::byte bytes[value_size];
	stream_->read_exactly(bytes, value_size);
	const std::uint32_t checksum = endian::read_u32_le(bytes);
	if (checksum != crc32_.get_value())
	{
		BSTONE_THROW_STATIC_SOURCE("Checksum mismatch.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_bool(bool value)
{
	write_uint8(value);
}

void Archiver::write_int8(std::int8_t value)
{
	write_uint8(static_cast<std::uint8_t>(value));
}

void Archiver::write_uint8(std::uint8_t value)
try {
	BSTONE_ASSERT(is_open());
	constexpr int value_size = 1;
	crc32_.update(&value, value_size);
	stream_->write_exactly(&value, value_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_int16(std::int16_t value)
{
	write_uint16(static_cast<std::uint16_t>(value));
}

void Archiver::write_uint16(std::uint16_t value)
try {
	BSTONE_ASSERT(is_open());
	constexpr int value_size = 2;
	std::byte bytes[value_size];
	endian::write_u16_le(value, bytes);
	crc32_.update(bytes, value_size);
	stream_->write_exactly(bytes, value_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_int32(std::int32_t value)
{
	write_uint32(static_cast<std::uint32_t>(value));
}

void Archiver::write_uint32(std::uint32_t value)
try {
	BSTONE_ASSERT(is_open());
	constexpr int value_size = 4;
	std::byte bytes[value_size];
	endian::write_u32_le(value, bytes);
	crc32_.update(bytes, value_size);
	stream_->write_exactly(bytes, value_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_char_array(const char* items, int item_count)
try {
	write_uint8_array(reinterpret_cast<const std::uint8_t*>(items), item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_int8_array(const std::int8_t* items, int item_count)
{
	write_uint8_array(reinterpret_cast<const std::uint8_t*>(items), item_count);
}

void Archiver::write_uint8_array(const std::uint8_t* items, int item_count)
try {
	BSTONE_ASSERT(is_open());
	BSTONE_ASSERT(items != nullptr);
	BSTONE_ASSERT(item_count > 0);
	crc32_.update(items, item_count);
	stream_->write_exactly(items, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_uint16_array(const std::uint16_t* items, int item_count)
try {
	constexpr int item_size = 2;
	const int items_size = item_count * item_size;
	constexpr int max_buffer_size = 128;
	constexpr int items_per_buffer = max_buffer_size / item_size;
	std::byte buffer[max_buffer_size];
	int item_offset = 0;
	while (item_count > 0)
	{
		const int to_write_count = std::min(item_count, items_per_buffer);
		for (int i = 0; i < to_write_count; ++i)
		{
			endian::write_u16_le(items[item_offset + i], buffer + i * item_size);
		}
		const int to_write_size = to_write_count * item_size;
		crc32_.update(buffer, to_write_size);
		stream_->write_exactly(buffer, to_write_size);
		item_count -= to_write_count;
		item_offset += to_write_count;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_string(const char* string, int string_length)
try {
	BSTONE_ASSERT(string != nullptr);
	BSTONE_ASSERT(string_length >= 0 && string_length <= INT32_MAX);
	write_int32(static_cast<std::int32_t>(string_length));
	stream_->write_exactly(string, string_length);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_checksum()
try {
	BSTONE_ASSERT(is_open());
	constexpr int value_size = 4;
	std::byte bytes[value_size];
	endian::write_u32_le(crc32_.get_value(), bytes);
	stream_->write_exactly(bytes, value_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
