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
class MemoryStream final : public Stream
{
public:
	MemoryStream(int initial_capacity = 0, StreamOpenMode open_mode = StreamOpenMode::read_write) noexcept;

	MemoryStream(
		int buffer_size,
		int buffer_offset,
		const unsigned char* buffer,
		StreamOpenMode open_mode = StreamOpenMode::read) noexcept;

	MemoryStream(const MemoryStream& rhs) = delete;
	MemoryStream(MemoryStream&& rhs) noexcept;
	MemoryStream& operator=(const MemoryStream& rhs) = delete;
	virtual ~MemoryStream();

	bool open(int initial_capacity = 0,StreamOpenMode open_mode = StreamOpenMode::read_write) noexcept;

	bool open(
		int buffer_size,
		int buffer_offset,
		const unsigned char* buffer,
		StreamOpenMode open_mode = StreamOpenMode::read) noexcept;

	void close() noexcept override;
	bool is_open() const noexcept override;
	int get_size() noexcept override;
	bool set_size(int size) noexcept override;
	int seek(int offset, StreamSeekOrigin origin) noexcept override;
	int get_position() noexcept override;
	int read(void* buffer, int count) noexcept override;
	bool write(const void* buffer, int count) noexcept override;
	bool flush() noexcept override;
	bool is_readable() const noexcept override;
	bool is_seekable() const noexcept override;
	bool is_writable() const noexcept override;
	unsigned char* get_data() noexcept;
	const unsigned char* get_data() const noexcept;
	bool remove_block(int offset, int count) noexcept;

private:
	using Buffer = std::vector<UnValue<std::uint8_t>>;

	bool is_open_{};
	bool is_readable_{};
	bool is_writable_{};
	int position_{};
	int size_{};
	int ext_size_{};
	unsigned char* buffer_{};
	unsigned char* ext_buffer_{};
	Buffer int_buffer_{};

	void close_internal() noexcept;
}; // Stream

} // bstone

#endif // !BSTONE_MEMORY_STREAM_INCLUDED
