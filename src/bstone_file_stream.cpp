/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File stream.

#include "bstone_exception.h"
#include "bstone_file_stream.h"

namespace bstone {

FileStream::FileStream(const char* file_name, FileOpenMode open_mode)
	:
	file_{file_name, open_mode}
{}

void FileStream::open(const char* file_name, FileOpenMode open_mode)
{
	file_.open(file_name, open_mode);
}

void FileStream::do_close()
{
	file_.close();
}

bool FileStream::do_is_open() const
{
	return file_.is_open();
}

IntP FileStream::do_read(void* buffer, IntP count)
{
	return file_.read(buffer, count);
}

IntP FileStream::do_write(const void* buffer, IntP count)
{
	return file_.write(buffer, count);
}

Int64 FileStream::do_seek(Int64 offset, StreamOrigin origin)
try {
	auto file_origin = FileOrigin::none;

	switch (origin)
	{
		case StreamOrigin::begin: file_origin = FileOrigin::begin; break;
		case StreamOrigin::current: file_origin = FileOrigin::current; break;
		case StreamOrigin::end: file_origin = FileOrigin::end; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown origin.");
	}

	return file_.seek(offset, file_origin);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Int64 FileStream::do_get_size() const
{
	return file_.get_size();
}

void FileStream::do_set_size(Int64 size)
{
	file_.set_size(size);
}

void FileStream::do_flush()
{
	file_.flush();
}

} // namespace bstone
