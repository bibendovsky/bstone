/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_VFS_INCLUDED
#define BSTONE_VFS_INCLUDED

#include "bstone_logger.h"
#include <memory>
#include <span>
#include <string_view>

namespace bstone {

enum class VfsSearchPathType
{
	unknown = 0,
	directory,
	archive,
};

struct VfsSearchPath
{
	VfsSearchPathType type;
	const char* path;
};

class VfsInputStream
{
public:
	VfsInputStream() = default;
	virtual ~VfsInputStream() = default;

	virtual int get_pathname_index() const = 0;
	virtual int get_size() = 0;
	virtual bool rewind() = 0;
	virtual int read(void* buffer, int buffer_size) = 0;

	virtual bool read_exactly(void* buffer, int buffer_size)
	{
		const auto bytes = static_cast<unsigned char*>(buffer);
		int bytes_offset = 0;
		while (bytes_offset < buffer_size)
		{
			const int read_size = read(bytes + bytes_offset, buffer_size - bytes_offset);
			if (read_size <= 0)
				break;
			bytes_offset += read_size;
		}
		return bytes_offset == buffer_size;
	}
};

using VfsInputStreamUPtr = std::unique_ptr<VfsInputStream>;

struct VfsInitParam
{
	Logger* logger;
	std::span<const char*> search_paths;
};

class Vfs
{
public:
	Vfs() = default;
	virtual ~Vfs() = default;

	virtual bool is_initialized() const = 0;
	virtual void terminate() = 0;
	virtual bool initialize(const VfsInitParam& param) = 0;
	virtual int get_search_path_count() = 0;
	virtual const VfsSearchPath& get_search_path(int index) = 0;
	virtual VfsInputStreamUPtr open_file(std::string_view vfs_pathname) = 0;
	virtual VfsInputStreamUPtr open_file(const VfsSearchPath& search_path, std::string_view vfs_pathname) = 0;
	virtual VfsInputStreamUPtr open_any_file(std::span<std::string_view> vfs_pathnames) = 0;
	virtual VfsInputStreamUPtr open_any_file(const VfsSearchPath& search_path, std::span<std::string_view> vfs_pathnames) = 0;
};

using VfsUPtr = std::unique_ptr<Vfs>;

VfsUPtr make_vfs();

} // namespace bstone

#endif // BSTONE_VFS_INCLUDED
