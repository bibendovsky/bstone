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
// File stream.
//


#ifndef BSTONE_FILE_STREAM_INCLUDED
#define BSTONE_FILE_STREAM_INCLUDED


#include "bstone_stream.h"


namespace bstone
{


// File stream.
class FileStream final :
	public Stream
{
public:
	FileStream();

	FileStream(
		const std::string& file_name,
		const StreamOpenMode open_mode = StreamOpenMode::read);

	FileStream(
		const FileStream& rhs) = delete;

	FileStream(
		FileStream&& rhs);

	FileStream& operator=(
		const FileStream& rhs) = delete;

	virtual ~FileStream();


	bool open(
		const std::string& file_name,
		const StreamOpenMode open_mode = StreamOpenMode::read);

	void close() override;

	bool is_open() const override;

	std::int64_t get_size() override;

	bool set_size(
		const std::int64_t size) override;

	std::int64_t seek(
		const std::int64_t offset,
		const StreamSeekOrigin origin) override;

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


	static bool is_exists(
		const std::string& file_name);


private:
	void* context_;
	bool is_readable_;
	bool is_seekable_;
	bool is_writable_;


	void close_internal();
}; // FileStream


} // bstone


#endif // BSTONE_FILE_STREAM_INCLUDED
