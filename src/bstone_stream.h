/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// A base class for streams.
//

#ifndef BSTONE_STREAM_INCLUDED
#define BSTONE_STREAM_INCLUDED

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
	Stream() noexcept = default;
	virtual ~Stream() = default;

	virtual void close() noexcept = 0;
	virtual bool is_open() const noexcept = 0;

	// Returns a size of the stream or a negative value on error.
	virtual int get_size() noexcept = 0;

	// Sets a new size of the stream.
	// Returns false on error or if the stream is not seekable.
	virtual bool set_size(int size) noexcept = 0;

	// Returns a new position or a negative value on error.
	virtual int seek(int offset, StreamSeekOrigin origin) noexcept = 0;

	// Skips a number of octets forward if count is positive or
	// backward otherwise.
	// Returns a negative value on error.
	virtual int skip(int count) noexcept;

	// Returns a current position or a negative value on error.
	virtual int get_position() noexcept;

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
	virtual bool set_position(int position) noexcept;

	// Reads a specified number of octets and returns an actual
	// read number of octets.
	virtual int read(void* buffer, int count) noexcept = 0;

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
	virtual bool write(const void* buffer, int count) noexcept = 0;

	// Reads a one octet and returns it.
	// Returns a negative value on error.
	virtual int read_octet() noexcept;

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
	virtual bool write_octet(unsigned char value) noexcept;

	// Writes a string without a terminator
	virtual bool write_string(const std::string& string) noexcept;

	//
	// Flushes the buffers.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	virtual bool flush() noexcept = 0;

	//
	// Indicates the stream is readable.
	//
	// Returns:
	//    - "true" if stream is readable.
	//    - "false" otherwise.
	//
	virtual bool is_readable() const noexcept = 0;

	//
	// Indicates the stream is seekable.
	//
	// Returns:
	//    - "true" if stream is seekable.
	//    - "false" otherwise.
	//
	virtual bool is_seekable() const noexcept = 0;

	//
	// Indicates the stream is writable.
	//
	// Returns:
	//    - "true" if stream is writable.
	//    - "false" otherwise.
	//
	virtual bool is_writable() const noexcept = 0;

	// Copies the stream from a current position to
	// an another stream using internal buffer.
	bool copy_to(Stream* dst_stream, int buffer_size = 0) noexcept;


	//
	// Gets default buffer size for copying.
	//
	// Returns:
	//    - Default buffer size.
	//
	static int get_default_copy_buffer_size() noexcept;
}; // Stream


} // bstone

#endif // BSTONE_STREAM_INCLUDED
