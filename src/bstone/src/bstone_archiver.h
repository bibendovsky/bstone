/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_ARCHIVER_INCLUDED
#define BSTONE_ARCHIVER_INCLUDED

#include "bstone_crc32.h"
#include "bstone_stream.h"
#include <cstdint>

namespace bstone {

class Archiver
{
public:
	void open(Stream& stream);
	void close();
	bool is_open() const;

	bool read_bool();
	std::int8_t read_int8();
	std::uint8_t read_uint8();
	std::int16_t read_int16();
	std::uint16_t read_uint16();
	std::int32_t read_int32();
	std::uint32_t read_uint32();
	void read_char_array(char* items, int item_count);
	void read_int8_array(std::int8_t* items, int item_count);
	void read_uint8_array(std::uint8_t* items, int item_count);
	void read_uint16_array(std::uint16_t* items, int item_count);
	int read_string(int max_string_length, char* string);
	void read_checksum();

	void write_bool(bool value);
	void write_int8(std::int8_t value);
	void write_uint8(std::uint8_t value);
	void write_int16(std::int16_t value);
	void write_uint16(std::uint16_t value);
	void write_int32(std::int32_t value);
	void write_uint32(std::uint32_t value);
	void write_char_array(const char* items, int item_count);
	void write_int8_array(const std::int8_t* items, int item_count);
	void write_uint8_array(const std::uint8_t* items, int item_count);
	void write_uint16_array(const std::uint16_t* items, int item_count);
	void write_string(const char* string, int string_length);
	void write_checksum();

private:
	Crc32 crc32_{};
	Stream* stream_{};
};

} // namespace bstone

#endif // BSTONE_ARCHIVER_INCLUDED
