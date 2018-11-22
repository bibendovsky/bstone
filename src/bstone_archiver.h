/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#include <memory>


namespace bstone
{


class Stream;
using StreamPtr = Stream*;


class Archiver
{
public:
	virtual void destroy() = 0;


	virtual void initialize(
		StreamPtr stream) = 0;

	virtual void uninitialize() = 0;

	virtual bool is_initialized() const = 0;


	virtual char read_char() = 0;

	virtual signed char read_schar() = 0;

	virtual unsigned char read_uchar() = 0;


	virtual void read_char_array(
		char* items,
		const int item_count) = 0;

	virtual void read_schar_array(
		signed char* items,
		const int item_count) = 0;

	virtual void read_uchar_array(
		unsigned char* items,
		const int item_count) = 0;


	virtual short read_sshort() = 0;

	virtual unsigned short read_ushort() = 0;


	virtual long read_slong() = 0;

	virtual unsigned long read_ulong() = 0;


	virtual void read_sshort_array(
		short* items,
		const int item_count) = 0;

	virtual void read_ushort_array(
		unsigned short* items,
		const int item_count) = 0;


	virtual void read_string(
		const int max_string_length,
		char* const string,
		int& string_length) = 0;


	virtual void read_checksum() = 0;


	virtual void write_char(
		const char value_char) = 0;

	virtual void write_schar(
		const signed char value_schar) = 0;

	virtual void write_uchar(
		const unsigned char value_uchar) = 0;


	virtual void write_char_array(
		const char* const items,
		const int item_count) = 0;

	virtual void write_schar_array(
		const signed char* const items,
		const int item_count) = 0;

	virtual void write_uchar_array(
		const unsigned char* const items,
		const int item_count) = 0;


	virtual void write_sshort(
		const short value_sshort) = 0;

	virtual void write_ushort(
		const unsigned short value_ushort) = 0;


	virtual void write_slong(
		const long value_slong) = 0;

	virtual void write_ulong(
		const unsigned long value_ulong) = 0;



	virtual void write_sshort_array(
		const short* const items,
		const int item_count) = 0;

	virtual void write_ushort_array(
		const unsigned short* const items,
		const int item_count) = 0;


	virtual void write_string(
		const char* const string,
		const int string_length) = 0;


	virtual void write_checksum() = 0;


protected:
	Archiver() = default;

	virtual ~Archiver() = default;
}; // Archiver


using ArchiverPtr = Archiver*;


class ArchiverException
{
public:
	virtual const char* get_message() const = 0;


protected:
	ArchiverException() = default;

	virtual ~ArchiverException() = default;
}; // ArchiverException


struct ArchiverDeleter
{
	void operator()(
		ArchiverPtr archiver)
	{
		archiver->destroy();
	}
}; // ArchiverDeleter


using ArchiverUPtr = std::unique_ptr<Archiver, ArchiverDeleter>;


struct ArchiverFactory final
{
	static ArchiverUPtr create();
}; // ArchiverFactory


} // bstone


#endif // !BSTONE_ARCHIVER_INCLUDED
