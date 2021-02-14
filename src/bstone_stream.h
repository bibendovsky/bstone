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


//
// A base class for streams.
//


#ifndef BSTONE_STREAM_INCLUDED
#define BSTONE_STREAM_INCLUDED


#include <cstdint>

#include <string>


namespace bstone
{


enum class StreamSeekOrigin
{
	begin,
	current,
	end,
}; // StreamSeekOrigin

enum class StreamOpenMode
{
	read,
	write,
	read_write,
}; // StreamOpenMode


// A base class for streams.
class Stream
{
public:
	Stream();

	virtual ~Stream();


	virtual void close();

	virtual bool is_open() const = 0;

	// Returns a size of the stream or a negative value on error.
	virtual std::int64_t get_size() = 0;

	// Sets a new size of the stream.
	// Returns false on error or if the stream is not seekable.
	virtual bool set_size(
		const std::int64_t size) = 0;

	// Returns a new position or a negative value on error.
	virtual std::int64_t seek(
		const std::int64_t offset,
		const StreamSeekOrigin origin) = 0;

	// Skips a number of octets forward if count is positive or
	// backward otherwise.
	// Returns a negative value on error.
	virtual std::int64_t skip(
		const int count);

	// Returns a current position or a negative value on error.
	virtual std::int64_t get_position();

	//
	// Moves stream's pointer to a new position.
	//
	// Parameters:
	//    - position - new position.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	virtual bool set_position(
		const std::int64_t position);

	// Reads a specified number of octets and returns an actual
	// read number of octets.
	virtual int read(
		void* buffer,
		const int count) = 0;

	//
	// Writes octet data into the stream.
	//
	// Parameters:
	//    - buffer - buffer with octet data.
	//    - count - octet count.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	virtual bool write(
		const void* buffer,
		const int count) = 0;

	// Reads a one octet and returns it.
	// Returns a negative value on error.
	virtual int read_octet();

	//
	// Writes exactly one octet into the stream.
	//
	// Parameters:
	//    - value - octet value.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	virtual bool write_octet(
		const std::uint8_t value);

	// Writes a string without a terminator
	virtual bool write_string(
		const std::string& string);

	//
	// Flushes the buffers.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	virtual bool flush() = 0;

	//
	// Indicates the stream is readable.
	//
	// Returns:
	//    - "true" if stream is readable.
	//    - "false" otherwise.
	//
	virtual bool is_readable() const = 0;

	//
	// Indicates the stream is seekable.
	//
	// Returns:
	//    - "true" if stream is seekable.
	//    - "false" otherwise.
	//
	virtual bool is_seekable() const = 0;

	//
	// Indicates the stream is writable.
	//
	// Returns:
	//    - "true" if stream is writable.
	//    - "false" otherwise.
	//
	virtual bool is_writable() const = 0;

	// Copies the stream from a current position to
	// an another stream using internal buffer.
	bool copy_to(
		Stream* dst_stream,
		const int buffer_size = 0);


	//
	// Gets default buffer size for copying.
	//
	// Returns:
	//    - Default buffer size.
	//
	static int get_default_copy_buffer_size();
}; // Stream


} // bstone


#endif // BSTONE_STREAM_INCLUDED
