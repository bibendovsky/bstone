/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File stream.

#include "bstone_file_stream.h"

#include "bstone_exception.h"

namespace bstone {

FileStream::FileStream() = default;

FileStream::FileStream(const char* path, sys::FileMode mode)
	:
	file_(path, mode)
{}

FileStream::FileStream(FileStream&&) noexcept = default;

FileStream& FileStream::operator=(FileStream&&) noexcept = default;

FileStream::~FileStream() = default;

bool FileStream::open(const char* path, sys::FileMode mode)
{
	return file_.open(path, mode);
}

void FileStream::do_close()
{
	file_.close();
}

bool FileStream::do_is_open() const
{
	return file_.is_open();
}

std::intptr_t FileStream::do_read(void* buffer, std::intptr_t count)
try {
	const std::intptr_t result = file_.read(buffer, static_cast<int>(count));

	if (result < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to read.");
	}

	return result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::intptr_t FileStream::do_write(const void* buffer, std::intptr_t count)
try {
	const std::intptr_t result = file_.write(buffer, static_cast<int>(count));

	if (result < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to write.");
	}

	return result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::int64_t FileStream::do_seek(std::int64_t offset, StreamOrigin origin)
try {
	sys::FileOrigin file_origin;

	switch (origin)
	{
		case StreamOrigin::begin: file_origin = sys::FileOrigin::begin; break;
		case StreamOrigin::current: file_origin = sys::FileOrigin::current; break;
		case StreamOrigin::end: file_origin = sys::FileOrigin::end; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown origin.");
	}

	const std::int64_t result = file_.seek(offset, file_origin);

	if (result < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to seek.");
	}

	return result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::int64_t FileStream::do_get_size()
try {
	const std::int64_t result = file_.get_size();

	if (result < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get a size.");
	}

	return result;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void FileStream::do_set_size([[maybe_unused]] std::int64_t size)
try {
	BSTONE_THROW_STATIC_SOURCE("Resize not supported.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void FileStream::do_flush()
try {
	if (!file_.flush())
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to flush.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
