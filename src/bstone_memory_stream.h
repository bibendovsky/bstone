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
// A memory stream.
//


#ifndef BSTONE_MEMORY_STREAM_INCLUDED
#define BSTONE_MEMORY_STREAM_INCLUDED


#include <vector>

#include "bstone_stream.h"
#include "bstone_un_value.h"


namespace bstone
{


// A memory stream.
class MemoryStream final :
	public Stream
{
public:
	MemoryStream(
		const int initial_capacity = 0,
		const StreamOpenMode open_mode = StreamOpenMode::read_write);

	MemoryStream(
		const int buffer_size,
		const int buffer_offset,
		const std::uint8_t* buffer,
		const StreamOpenMode open_mode = StreamOpenMode::read);

	MemoryStream(
		const MemoryStream& rhs) = delete;

	MemoryStream(
		MemoryStream&& rhs);

	MemoryStream& operator=(
		const MemoryStream& rhs) = delete;

	virtual ~MemoryStream();


	bool open(
		const int initial_capacity = 0,
		const StreamOpenMode open_mode = StreamOpenMode::read_write);

	bool open(
		const int buffer_size,
		const int buffer_offset,
		const std::uint8_t* buffer,
		const StreamOpenMode open_mode = StreamOpenMode::read);

	void close() override;

	bool is_open() const override;

	std::int64_t get_size() override;

	bool set_size(
		const std::int64_t size) override;

	std::int64_t seek(
		const std::int64_t offset,
		const StreamSeekOrigin origin) override;

	std::int64_t get_position() override;

	int read(
		void* buffer,
		const int count) override;

	bool write(
		const void* buffer,
		const int count) override;

	bool flush() override;

	bool is_readable() const override;

	bool is_seekable() const override;

	bool is_writable() const override;

	std::uint8_t* get_data();

	const std::uint8_t* get_data() const;

	bool remove_block(
		const std::int64_t offset,
		const int count);


private:
	using Buffer = std::vector<UnValue<std::uint8_t>>;


	bool is_open_;
	bool is_readable_;
	bool is_writable_;
	std::int64_t position_;
	std::int64_t size_;
	std::int64_t ext_size_;
	std::uint8_t* buffer_;
	std::uint8_t* ext_buffer_;
	Buffer int_buffer_;


	void close_internal();
}; // Stream


} // bstone


#endif // !BSTONE_MEMORY_STREAM_INCLUDED
