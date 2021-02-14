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


#ifndef BSTONE_ARCHIVER_INCLUDED
#define BSTONE_ARCHIVER_INCLUDED


#include <cstdint>

#include <memory>

#include "bstone_exception.h"


namespace bstone
{


class Stream;
using StreamPtr = Stream*;


class ArchiverException :
	public Exception
{
public:
	explicit ArchiverException(
		const char* const message);

	virtual ~ArchiverException();
}; // ArchiverException


class Archiver
{
public:
	Archiver() = default;

	virtual ~Archiver() = default;


	virtual void initialize(
		StreamPtr stream) = 0;

	virtual void uninitialize() = 0;

	virtual bool is_initialized() const = 0;


	virtual void throw_exception(
		const char* const message) const = 0;


	virtual bool read_bool() = 0;

	virtual char read_char() = 0;


	virtual std::int8_t read_int8() = 0;

	virtual std::uint8_t read_uint8() = 0;


	virtual std::int16_t read_int16() = 0;

	virtual std::uint16_t read_uint16() = 0;


	virtual std::int32_t read_int32() = 0;

	virtual std::uint32_t read_uint32() = 0;


	virtual void read_char_array(
		char* items_char,
		const int item_count) = 0;

	virtual void read_int8_array(
		std::int8_t* items_int8,
		const int item_count) = 0;

	virtual void read_uint8_array(
		std::uint8_t* items_uint8,
		const int item_count) = 0;


	virtual void read_int16_array(
		std::int16_t* items_int16,
		const int item_count) = 0;

	virtual void read_uint16_array(
		std::uint16_t* items_uint16,
		const int item_count) = 0;


	virtual void read_string(
		const int max_string_length,
		char* const string,
		int& string_length) = 0;


	virtual void read_checksum() = 0;


	virtual void write_bool(
		const bool value_bool) = 0;

	virtual void write_char(
		const char value_char) = 0;


	virtual void write_int8(
		const std::int8_t value_int8) = 0;

	virtual void write_uint8(
		const std::uint8_t value_uint8) = 0;


	virtual void write_int16(
		const std::int16_t value_int16) = 0;

	virtual void write_uint16(
		const std::uint16_t value_uint16) = 0;


	virtual void write_int32(
		const std::int32_t value_int32) = 0;

	virtual void write_uint32(
		const std::uint32_t value_uint32) = 0;


	virtual void write_char_array(
		const char* const items_char,
		const int item_count) = 0;

	virtual void write_int8_array(
		const std::int8_t* const items_int8,
		const int item_count) = 0;

	virtual void write_uint8_array(
		const std::uint8_t* const items_uint8,
		const int item_count) = 0;


	virtual void write_int16_array(
		const std::int16_t* const items_int16,
		const int item_count) = 0;

	virtual void write_uint16_array(
		const std::uint16_t* const items_uint16,
		const int item_count) = 0;


	virtual void write_string(
		const char* const string,
		const int string_length) = 0;


	virtual void write_checksum() = 0;
}; // Archiver


using ArchiverUPtr = std::unique_ptr<Archiver>;


ArchiverUPtr make_archiver();


} // bstone


#endif // !BSTONE_ARCHIVER_INCLUDED
