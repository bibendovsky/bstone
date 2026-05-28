/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_archive_file_input_stream.h"
#include "bstone_assert.h"
#include "bstone_sys_file.h"
#include <string>

namespace bstone {

namespace {

class ArchiveFileFileInputStream final : public ArchiveFileInputStream
{
public:
	explicit ArchiveFileFileInputStream(const char* file_path);
	~ArchiveFileFileInputStream() override = default;

	ArchiveFileInputStream* open_from_this() override;
	bool is_open() const override;
	std::int64_t get_size() override;
	bool set_position(std::int64_t position) override;
	std::int64_t skip(std::int64_t count) override;
	int read(void* buffer, int count) override;
	bool read_exactly(void* buffer, int count) override;

private:
	std::string file_path_{};
	sys::File file_{};

private:
	bool impl_is_open() const;
};

// --------------------------------------

ArchiveFileFileInputStream::ArchiveFileFileInputStream(const char* file_path)
{
	if (file_.open(file_path, sys::FileMode::read))
		file_path_ = file_path;
}

ArchiveFileInputStream* ArchiveFileFileInputStream::open_from_this()
{
	BSTONE_ASSERT(impl_is_open());
	return new ArchiveFileFileInputStream(file_path_.c_str());
}

bool ArchiveFileFileInputStream::is_open() const
{
	return impl_is_open();
}

std::int64_t ArchiveFileFileInputStream::get_size()
{
	return file_.get_size();
}

bool ArchiveFileFileInputStream::set_position(std::int64_t position)
{
	return file_.set_position(position);
}

std::int64_t ArchiveFileFileInputStream::skip(std::int64_t count)
{
	return file_.skip(count);
}

int ArchiveFileFileInputStream::read(void* buffer, int count)
{
	return file_.read(buffer, count);
}

bool ArchiveFileFileInputStream::read_exactly(void* buffer, int count)
{
	return file_.read_exactly(buffer, count);
}

bool ArchiveFileFileInputStream::impl_is_open() const
{
	return file_.is_open();
}

} // namespace

// ======================================

ArchiveFileInputStreamUPtr make_archive_file_file_input_stream(const char* file_path)
{
	BSTONE_ASSERT(file_path != nullptr);
	return std::make_unique<ArchiveFileFileInputStream>(file_path);
}

} // namespace bstone
