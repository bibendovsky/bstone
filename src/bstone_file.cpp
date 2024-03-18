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
		BSTONE_THROW_STATIC_SOURCE("Data underflow.");
	}
}

void File::write_exact(const void* buffer, std::intptr_t count) const
{
	if (write(buffer, count) != count)
	{
		BSTONE_THROW_STATIC_SOURCE("Data overflow.");
	}
}

std::int64_t File::skip(std::int64_t delta) const
{
	return seek(delta, FileOrigin::current);
}

std::int64_t File::get_position() const
{
	return skip(0);
}

void File::set_position(std::int64_t position) const
{
	ensure_position_not_negative(position);
	seek(position, FileOrigin::begin);
}

void File::ensure_is_open() const
{
	if (!is_open())
	{
		BSTONE_THROW_STATIC_SOURCE("Not open.");
	}
}

void File::ensure_path_not_null(const char* path)
{
	if (path == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null path.");
	}
}

void File::ensure_buffer_not_null(const void* buffer)
{
	if (buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null buffer.");
	}
}

void File::ensure_count_not_negative(std::intptr_t count)
{
	if (count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative count.");
	}
}

void File::ensure_position_not_negative(std::int64_t position)
{
	if (position < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative position.");
	}
}

void File::ensure_size_not_negative(std::int64_t size)
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
