/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive.

#include "bstone_file.h"

#include "bstone_exception.h"

namespace bstone {

File::File() noexcept = default;

File::File(const char* path, FileOpenFlags open_flags, FileShareMode share_mode)
{
	try_or_open_internal(path, open_flags, share_mode, FileErrorMode::exception, resource_);
}

bool File::try_open(const char* path, FileOpenFlags open_flags, FileShareMode share_mode)
{
	return try_or_open_internal(path, open_flags, share_mode, FileErrorMode::error_code, resource_);
}

void File::open(const char* path, FileOpenFlags open_flags, FileShareMode share_mode)
{
	try_or_open_internal(path, open_flags, share_mode, FileErrorMode::exception, resource_);
}

void File::close() noexcept
{
	close_internal(resource_);
}

bool File::is_open() const noexcept
{
	return !resource_.is_empty();
}

void File::read_exactly(void* buffer, std::intptr_t count) const
{
	auto buffer_bytes = static_cast<unsigned char*>(buffer);

	while (true)
	{
		const auto read_count = read(buffer_bytes, count);

		if (read_count == 0)
		{
			if (count != 0)
			{
				BSTONE_THROW_STATIC_SOURCE("Read number of bytes mismatch.");
			}

			break;
		}

		buffer_bytes += read_count;
		count -= read_count;
	}
}

void File::write_exactly(const void* buffer, std::intptr_t count) const
{
	auto buffer_bytes = static_cast<const unsigned char*>(buffer);

	while (true)
	{
		const auto written_count = write(buffer_bytes, count);

		if (written_count == 0)
		{
			if (count != 0)
			{
				BSTONE_THROW_STATIC_SOURCE("Written number of bytes mismatch.");
			}

			break;
		}

		buffer_bytes += written_count;
		count -= written_count;
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
	seek(position, FileOrigin::begin);
}

void File::close_internal(FileUResource& resource) noexcept
{
	resource.reset();
}

} // namespace bstone
