/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive.

#include "bstone_file.h"

#include "bstone_exception.h"
#include "bstone_utility.h"

namespace bstone {

File::File(const char* path, FileOpenFlags open_flags)
{
	try_or_open_internal(path, open_flags, FileErrorMode::exception, resource_);
}

bool File::try_open(const char* path, FileOpenFlags open_flags)
{
	return try_or_open_internal(path, open_flags, FileErrorMode::error_code, resource_);
}

void File::open(const char* path, FileOpenFlags open_flags)
{
	try_or_open_internal(path, open_flags, FileErrorMode::exception, resource_);
}

void File::close() noexcept
{
	close_internal(resource_);
}

bool File::is_open() const noexcept
{
	return !resource_.is_empty();
}

void File::read_exact(void* buffer, std::intptr_t count) const
{
	if (read(buffer, count) != count)
	{
		BSTONE_THROW_STATIC_SOURCE("Read number of bytes mismatch.");
	}
}

void File::write_exact(const void* buffer, std::intptr_t count) const
{
	if (write(buffer, count) != count)
	{
		BSTONE_THROW_STATIC_SOURCE("Written number of bytes mismatch.");
	}
}

std::int64_t File::skip(std::int64_t offset) const
{
	return seek(offset, FileOrigin::current);
}

std::int64_t File::get_position() const
{
	return skip(0);
}

void File::set_position(std::int64_t position) const
{
	validate_position(position);
	seek(position, FileOrigin::begin);
}

void File::ensure_is_open() const
{
	if (!is_open())
	{
		BSTONE_THROW_STATIC_SOURCE("Closed.");
	}
}

void File::validate_path(const char* path)
{
	if (path == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null path.");
	}
}

void File::validate_buffer(const void* buffer)
{
	if (buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null buffer.");
	}
}

void File::validate_count(std::intptr_t count)
{
	if (count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative count.");
	}
}

void File::validate_position(std::int64_t position)
{
	if (position < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative position.");
	}
}

void File::validate_size(std::int64_t size)
{
	if (size < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative size.");
	}
}

void File::close_internal(FileUResource& resource) noexcept
{
	resource.reset();
}

} // namespace bstone
