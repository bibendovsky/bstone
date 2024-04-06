/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_archiver.h"

namespace bstone {

void Archiver::open(Stream& stream)
try {
	close();

	stream_ = &stream;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::close() noexcept
{
	crc32_.reset();
	stream_ = nullptr;
}

bool Archiver::is_open() const noexcept
{
	return stream_ != nullptr;
}

bool Archiver::read_bool()
try {
	return read_integer<std::uint8_t>() != 0;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::int8_t Archiver::read_int8()
try {
	return read_integer<std::int8_t>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::uint8_t Archiver::read_uint8()
try {
	return read_integer<std::uint8_t>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::int16_t Archiver::read_int16()
try {
	return read_integer<std::int16_t>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::uint16_t Archiver::read_uint16()
try {
	return read_integer<std::uint16_t>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::int32_t Archiver::read_int32()
try {
	return read_integer<std::int32_t>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::uint32_t Archiver::read_uint32()
try {
	return read_integer<std::uint32_t>();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::read_char_array(char* items, std::intptr_t item_count)
try {
	read_integer_array(items, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::read_int8_array(std::int8_t* items_int8, std::intptr_t item_count)
try {
	read_integer_array(items_int8, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::read_uint8_array(std::uint8_t* items_uint8, std::intptr_t item_count)
try {
	read_integer_array(items_uint8, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::read_uint16_array(std::uint16_t* items_uint16, std::intptr_t item_count)
try {
	read_integer_array(items_uint16, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::intptr_t Archiver::read_string(std::intptr_t max_string_length, char* string)
try {
	BSTONE_ASSERT(max_string_length >= 0 && max_string_length <= INT32_MAX);
	BSTONE_ASSERT(string != nullptr);

	const auto archived_string_length = read_integer<std::int32_t>();

	if (archived_string_length < 0 || archived_string_length > max_string_length)
	{
		BSTONE_THROW_STATIC_SOURCE("Archived string length out of range.");
	}

	stream_->read_exactly(string, archived_string_length);
	return archived_string_length;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::read_checksum()
try {
	const auto checksum = read_integer<std::uint32_t>(true);

	if (checksum != crc32_.get_value())
	{
		BSTONE_THROW_STATIC_SOURCE("Checksum mismatch.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_bool(bool value)
try {
	write_integer<std::uint8_t>(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_int8(std::int8_t value)
try {
	write_integer(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_uint8(std::uint8_t value)
try {
	write_integer(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_int16(std::int16_t value)
try {
	write_integer(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_uint16(std::uint16_t value)
try {
	write_integer(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_int32(std::int32_t value)
try {
	write_integer(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_uint32(std::uint32_t value)
try {
	write_integer(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_char_array(const char* items_char, std::intptr_t item_count)
try {
	write_integer_array(items_char, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_int8_array(const std::int8_t* items_int8, std::intptr_t item_count)
try {
	write_integer_array(items_int8, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_uint8_array(const std::uint8_t* items_uint8, std::intptr_t item_count)
try {
	write_integer_array(items_uint8, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_uint16_array(const std::uint16_t* items_uint16, std::intptr_t item_count)
try {
	write_integer_array(items_uint16, item_count);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_string(const char* string, std::intptr_t string_length)
try {
	BSTONE_ASSERT(string != nullptr);
	BSTONE_ASSERT(string_length >= 0 && string_length <= INT32_MAX);

	write_integer<std::int32_t>(static_cast<std::int32_t>(string_length));
	stream_->write_exactly(string, string_length);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Archiver::write_checksum()
try {
	const auto checksum = crc32_.get_value();
	write_integer<std::int32_t>(checksum, true);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
