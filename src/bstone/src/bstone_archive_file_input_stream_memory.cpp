/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_archive_file_input_stream.h"
#include "bstone_assert.h"
#include <climits>
#include <algorithm>

namespace bstone {

namespace {

class ArchiveFileMemoryInputStream final : public ArchiveFileInputStream
{
public:
	ArchiveFileMemoryInputStream(const void* buffer, int count);
	~ArchiveFileMemoryInputStream() override = default;

	ArchiveFileInputStream* open_from_this() override;
	bool is_open() const override;
	std::int64_t get_size() override;
	bool set_position(std::int64_t position) override;
	std::int64_t skip(std::int64_t count) override;
	int read(void* buffer, int count) override;
	bool read_exactly(void* buffer, int count) override;

private:
	const std::byte* bytes_{};
	int max_size_{};
	int position_{};

	bool impl_is_open() const;
	int impl_read(void* buffer, int count);
};

// --------------------------------------

ArchiveFileMemoryInputStream::ArchiveFileMemoryInputStream(const void* buffer, int count)
	:
	bytes_{static_cast<const std::byte*>(buffer)},
	max_size_{count}
{}

ArchiveFileInputStream* ArchiveFileMemoryInputStream::open_from_this()
{
	BSTONE_ASSERT(impl_is_open());
	return new ArchiveFileMemoryInputStream(bytes_, max_size_);
}

bool ArchiveFileMemoryInputStream::is_open() const
{
	return impl_is_open();
}

std::int64_t ArchiveFileMemoryInputStream::get_size()
{
	return max_size_;
}

bool ArchiveFileMemoryInputStream::set_position(std::int64_t position)
{
	BSTONE_ASSERT(impl_is_open());
	BSTONE_ASSERT(position >= 0);
	if (position > INT_MAX)
		return false;
	position_ = static_cast<int>(position);
	return true;
}

std::int64_t ArchiveFileMemoryInputStream::skip(std::int64_t count)
{
	BSTONE_ASSERT(impl_is_open());
	const std::int64_t new_position = position_ + count;
	if (new_position < 0 || new_position > INT_MAX)
		return -1;
	position_ = static_cast<int>(new_position);
	return new_position;
}

int ArchiveFileMemoryInputStream::read(void* buffer, int count)
{
	BSTONE_ASSERT(impl_is_open());
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(count >= 0);
	return impl_read(buffer, count);
}

bool ArchiveFileMemoryInputStream::read_exactly(void* buffer, int count)
{
	BSTONE_ASSERT(impl_is_open());
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(count >= 0);
	return impl_read(buffer, count) == count;
}

bool ArchiveFileMemoryInputStream::impl_is_open() const
{
	return bytes_ != nullptr;
}

int ArchiveFileMemoryInputStream::impl_read(void* buffer, int count)
{
	const int to_read_size = std::min(max_size_ - position_, count);
	std::copy_n(bytes_ + position_, to_read_size, static_cast<std::byte*>(buffer));
	position_ += to_read_size;
	return to_read_size;
}

} // namespace

// ======================================

ArchiveFileInputStreamUPtr make_archive_file_memory_input_stream(const void* buffer, int count)
{
	BSTONE_ASSERT((buffer == nullptr && count == 0) || (buffer != nullptr && count >= 0));
	return std::make_unique<ArchiveFileMemoryInputStream>(buffer, count);
}

} // namespace bstone
