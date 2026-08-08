/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// References:
//  - Hash Functions
//    http://www.cse.yorku.ca/~oz/hash.html

#include "bstone_vfs.h"
#include "bstone_ascii.h"
#include "bstone_assert.h"
#include "bstone_fs_utils.h"
#include "bstone_sys_file.h"
#include "bstone_sys_fs.h"
#include "bstone_zip_archive_file.h"
#include <cstdint>
#include <algorithm>
#include <limits>
#include <memory>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace bstone {

namespace {

#define VFS_LOG_PREFIX "[VFS] "

bool is_valid_filename_char(char ch)
{
	// POSIX Portable Filename Character Set
	return
		(ch >= 'A' && ch <= 'Z') ||
		(ch >= 'a' && ch <= 'z') ||
		(ch >= '0' && ch <= '9') ||
		ch == '.' ||
		ch == '_' ||
		ch == '-';
}

bool is_archive_filename_character_valid(char ch)
{
	// [a-z\._-]
	return
		ascii::is_lower(ch) ||
		ascii::is_decimal(ch) ||
		ch == '.' ||
		ch == '_' ||
		ch == '-';
}

bool is_filename_character_valid(char ch)
{
	// [A-Za-z\._-]
	return
		ascii::is_upper(ch) ||
		ascii::is_lower(ch) ||
		ascii::is_decimal(ch) ||
		ch == '.' ||
		ch == '_' ||
		ch == '-';
}

bool is_archive_filename_valid(std::string_view filename)
{
	return std::all_of(filename.cbegin(), filename.cend(), &is_archive_filename_character_valid);
}

bool is_archive_entry_name_valid(std::string_view filename)
{
	return std::all_of(filename.cbegin(), filename.cend(), &is_filename_character_valid);
}

// =====================================

class VfsImplArchiveInputStream final : public VfsInputStream
{
public:
	VfsImplArchiveInputStream(ArchiveFileEntryStreamUPtr&& stream, int index);
	~VfsImplArchiveInputStream() override = default;

	int get_pathname_index() const override;
	int get_size() override;
	bool rewind() override;
	int read(void* buffer, int buffer_size) override;

private:
	ArchiveFileEntryStreamUPtr stream_{};
	int pathname_index_{-1};
};

// -------------------------------------

VfsImplArchiveInputStream::VfsImplArchiveInputStream(ArchiveFileEntryStreamUPtr&& stream, int pathname_index)
	:
	stream_{std::move(stream)},
	pathname_index_{pathname_index}
{}

int VfsImplArchiveInputStream::get_pathname_index() const
{
	return pathname_index_;
}

int VfsImplArchiveInputStream::get_size()
{
	return stream_->get_size();
}

bool VfsImplArchiveInputStream::rewind()
{
	return stream_->rewind();
}

int VfsImplArchiveInputStream::read(void* buffer, int buffer_size)
{
	return stream_->read(buffer, buffer_size);
}

// =====================================

class VfsImplFileInputStream final : public VfsInputStream
{
public:
	VfsImplFileInputStream(sys::File&& file, int index);
	~VfsImplFileInputStream() override = default;

	int get_pathname_index() const override;
	int get_size() override;
	bool rewind() override;
	int read(void* buffer, int buffer_size) override;

private:
	sys::File file_{};
	int pathname_index_{-1};
};

// -------------------------------------

VfsImplFileInputStream::VfsImplFileInputStream(sys::File&& file, int index)
	:
	file_{std::move(file)},
	pathname_index_{index}
{}

int VfsImplFileInputStream::get_pathname_index() const
{
	return pathname_index_;
}

int VfsImplFileInputStream::get_size()
{
	const long long size = file_.get_size();
	if (size < 0 || size > std::numeric_limits<int>::max())
		return -1;
	return static_cast<int>(size);
}

bool VfsImplFileInputStream::rewind()
{
	return file_.set_position(0);
}

int VfsImplFileInputStream::read(void* buffer, int buffer_size)
{
	return file_.read(buffer, buffer_size);
}

// =====================================

using VfsImplStringVector = std::vector<std::string>;

// relative pathname => entry index
using VfsSearchPathImplArchiveLookup = std::unordered_map<std::string_view, int>;

struct VfsSearchPathImpl : VfsSearchPath
{
	std::string impl_path;
	ArchiveFileUPtr impl_archive;
	VfsSearchPathImplArchiveLookup impl_archive_lookup;
};

using VfsSearchPathImplUPtr = std::unique_ptr<VfsSearchPathImpl>;

struct VfsFindArchivesContext
{
	const std::string_view archive_extension;
	VfsImplStringVector& result;

	sys::EnumDirCallbackResult callback(std::string_view filename)
	{
		std::string_view filename_without_extension;
		std::string_view extension;
		fs_utils::split_pathname_by_extension(filename, filename_without_extension, extension);
		if (is_archive_entry_name_valid(filename_without_extension) && extension == archive_extension)
			result.emplace_back(filename);
		return sys::EnumDirCallbackResult::resume;
	}

	static sys::EnumDirCallbackResult sys_callback(
		void* user_data,
		[[maybe_unused]] const char* directory_path,
		const char* file_name)
	{
		return static_cast<VfsFindArchivesContext*>(user_data)->callback(file_name);
	}
};

class VfsImpl final : public Vfs
{
public:
	~VfsImpl() override = default;

	bool is_initialized() const override;
	void terminate() override;
	bool initialize(const VfsInitParam& param) override;
	int get_search_path_count() override;
	const VfsSearchPath& get_search_path(int index) override;
	VfsInputStreamUPtr open_file(std::string_view vfs_pathname) override;
	VfsInputStreamUPtr open_file(const VfsSearchPath& search_path, std::string_view vfs_pathname) override;
	VfsInputStreamUPtr open_any_file(std::span<std::string_view> vfs_pathnames) override;
	VfsInputStreamUPtr open_any_file(const VfsSearchPath& search_path, std::span<std::string_view> vfs_pathnames) override;

private:
	inline constinit static const std::string_view log_prefix = "VFS";
	inline constinit static const std::string_view zip_extenstion = ".bstone_zip";

	// Every search path hands out a pointer into its own pathname, so the
	// entries have to keep their addresses as the list grows.
	using SearchPaths = std::vector<VfsSearchPathImplUPtr>;

	bool is_open_{};
	Logger* logger_{};
	SearchPaths search_paths_{};

	bool impl_is_initialized() const;
	void impl_terminate();
	bool impl_initialize(const VfsInitParam& param);
	bool is_valid_filename(std::string_view filename);
	bool is_valid_relative_pathname(std::string_view pathname);
	void make_archive_lookup(const ArchiveFile& archive, VfsSearchPathImplArchiveLookup& lookup);
	void add_search_path_archive(const std::string& pathname);
	void add_search_path_archives(const std::string& pathname);
	void add_search_path_directory(const std::string& pathname);
	void add_search_path(const char* path);
	void add_search_paths(const VfsInitParam& param);

	VfsInputStreamUPtr open_input_stream_archive(const VfsSearchPathImpl& search_path, std::span<std::string_view> vfs_pathname);
	VfsInputStreamUPtr open_input_stream_file(const VfsSearchPathImpl& search_path, std::span<std::string_view> vfs_pathname);
};

// --------------------------------------

bool VfsImpl::is_initialized() const
{
	return impl_is_initialized();
}

void VfsImpl::terminate()
{
	impl_terminate();
}

bool VfsImpl::initialize(const VfsInitParam& param)
{
	impl_terminate();
	if (!impl_initialize(param))
	{
		impl_terminate();
		return false;
	}
	return true;
}

int VfsImpl::get_search_path_count()
{
	return static_cast<int>(search_paths_.size());
}

const VfsSearchPath& VfsImpl::get_search_path(int index)
{
	BSTONE_ASSERT(index >= 0 && index < get_search_path_count());
	return *search_paths_[index];
}

VfsInputStreamUPtr VfsImpl::open_file(std::string_view vfs_pathname)
{
	std::string_view vfs_pathnames[1] = {vfs_pathname};
	return open_any_file(vfs_pathnames);
}

VfsInputStreamUPtr VfsImpl::open_file(const VfsSearchPath& search_path, std::string_view vfs_pathname)
{
	std::string_view vfs_pathnames[1] = {vfs_pathname};
	return open_any_file(search_path, vfs_pathnames);
}

VfsInputStreamUPtr VfsImpl::open_any_file(std::span<std::string_view> vfs_pathnames)
{
	for (const auto& search_path : std::views::reverse(search_paths_))
	{
		if (VfsInputStreamUPtr stream = open_any_file(*search_path, vfs_pathnames);
			stream != nullptr)
		{
			return stream;
		}
	}
	return nullptr;
}

VfsInputStreamUPtr VfsImpl::open_any_file(const VfsSearchPath& search_path, std::span<std::string_view> vfs_pathnames)
{
	const auto& search_path_impl = static_cast<const VfsSearchPathImpl&>(search_path);
	switch (search_path_impl.type)
	{
		case VfsSearchPathType::archive: return open_input_stream_archive(search_path_impl, vfs_pathnames);
		case VfsSearchPathType::directory: return open_input_stream_file(search_path_impl, vfs_pathnames);
		default: return nullptr;
	}
}

bool VfsImpl::impl_is_initialized() const
{
	return is_open_;
}

void VfsImpl::impl_terminate()
{
	is_open_ = false;
	logger_ = nullptr;
	search_paths_.clear();
}

bool VfsImpl::impl_initialize(const VfsInitParam& param)
{
	BSTONE_ASSERT(param.logger != nullptr);
	logger_ = param.logger;
	logger_->log_information("[{}] Initialize.", log_prefix);
	add_search_paths(param);
	is_open_ = true;
	return true;
}

bool VfsImpl::is_valid_filename(std::string_view filename)
{
	return
		!filename.empty() &&
		filename.front() != '.' &&
		std::all_of(filename.cbegin(), filename.cend(), &is_filename_character_valid);
}

bool VfsImpl::is_valid_relative_pathname(std::string_view pathname)
{
	if (pathname.empty())
		return false;
	std::size_t filename_index = 0;
	for (;;)
	{
		const std::size_t filename_index_end = pathname.find('/', filename_index);
		const std::string_view filename = pathname.substr(filename_index, filename_index_end - filename_index);
		if (!is_valid_filename(filename))
			return false;
		if (filename_index_end == std::string_view::npos)
			return true;
		filename_index = filename_index_end + 1;
	}
}

void VfsImpl::make_archive_lookup(const ArchiveFile& archive, VfsSearchPathImplArchiveLookup& lookup)
{
	const int entry_count = archive.get_entry_count();
	lookup.clear();
	lookup.reserve(static_cast<std::size_t>(entry_count));
	for (int i_entry = 0; i_entry < entry_count; ++i_entry)
	{
		const ArchiveFileEntry& entry = archive.get_entry(i_entry);
		const std::string_view name{entry.name, static_cast<std::size_t>(entry.name_length)};
		if (!is_valid_relative_pathname(name))
			continue;
		lookup.emplace(name, i_entry);
	}
}

void VfsImpl::add_search_path_archive(const std::string& pathname)
{
	logger_->log_information("[{}] Add archive \"{}\".", log_prefix, pathname);
	const std::string_view filename = fs_utils::get_filename(pathname);
	if (!is_valid_filename(filename))
		return;
	const std::string_view extension = fs_utils::get_extension(pathname);
	ArchiveFileUPtr archive{};
	if (extension == zip_extenstion)
		archive = make_zip_archive_file();
	else
	{
		logger_->log_error("[{}] Unsupported archive type.", log_prefix);
		return;
	}
	BSTONE_ASSERT(archive != nullptr);
	if (!archive->open_file(pathname.data()))
	{
		logger_->log_error("[{}] Failed to open.", log_prefix);
		return;
	}
	const int entry_count = archive->get_entry_count();
	logger_->log_information("[{}] Contains {} entries.", log_prefix, entry_count);
	if (entry_count == 0)
	{
		logger_->log_warning("[{}] No entries.", log_prefix);
		return;
	}
	VfsSearchPathImplArchiveLookup archive_lookup{};
	make_archive_lookup(*archive, archive_lookup);
	if (archive_lookup.empty())
	{
		logger_->log_warning("[{}] No compatible entries.", log_prefix);
		return;
	}
	logger_->log_information("[{}] Found {} compatible entries.", log_prefix, archive_lookup.size());
	VfsSearchPathImpl& search_path = *search_paths_.emplace_back(std::make_unique<VfsSearchPathImpl>());
	search_path.impl_path = pathname;
	search_path.impl_archive.swap(archive);
	search_path.impl_archive_lookup.swap(archive_lookup);
	search_path.type = VfsSearchPathType::archive;
	search_path.path = search_path.impl_path.data();
}

void VfsImpl::add_search_path_archives(const std::string& pathname)
{
	VfsImplStringVector archive_names{};
	VfsFindArchivesContext archive_context{
		.archive_extension = zip_extenstion,
		.result = archive_names};
	if (!sys::enumerate_directory(pathname.data(), &VfsFindArchivesContext::sys_callback, &archive_context))
		return;
	if (archive_names.empty())
		return;
	std::sort(archive_names.begin(), archive_names.end());
	std::string archive_path{};
	archive_path.reserve(pathname.size() + 128);
	for (const auto& archive_name : archive_names)
	{
		archive_path = pathname;
		fs_utils::append_path_inplace(archive_path, archive_name);
		add_search_path_archive(archive_path);
	}
}

void VfsImpl::add_search_path_directory(const std::string& pathname)
{
	// Add the archive files first.
	add_search_path_archives(pathname);
	// Add the directory.
	logger_->log_information("[{}] Add directory \"{}\".", log_prefix, pathname);
	sys::FileType file_type;
	sys::get_file_type(pathname.data(), file_type);
	if (file_type != sys::FileType::directory)
	{
		logger_->log_error("[{}] Unknown file type.", log_prefix);
		return;
	}
	VfsSearchPathImpl& search_path = *search_paths_.emplace_back(std::make_unique<VfsSearchPathImpl>());
	search_path.impl_path = pathname;
	search_path.type = VfsSearchPathType::directory;
	search_path.path = search_path.impl_path.data();
}

void VfsImpl::add_search_path(const char* path)
{
	BSTONE_ASSERT(path != nullptr);
	std::string new_path = path;
	fs_utils::trim_non_root_trailing_separator_inplace(new_path);
	logger_->log_information("[{}] Add search path \"{}\".", log_prefix, new_path);
	sys::FileType file_type;
	sys::get_file_type(path, file_type);
	if (file_type == sys::FileType::directory)
		add_search_path_directory(new_path);
	else
		logger_->log_error("[{}] Expected a directory.", log_prefix);
}

void VfsImpl::add_search_paths(const VfsInitParam& param)
{
	for (const auto& search_path : param.search_paths)
		add_search_path(search_path);
	logger_->log_information("[{}] Added {} search paths.", log_prefix, search_paths_.size());
}

VfsInputStreamUPtr VfsImpl::open_input_stream_archive(const VfsSearchPathImpl& search_path, std::span<std::string_view> vfs_pathnames)
{
	int vfs_pathname_index = 0;
	for (const auto& vfs_pathname : vfs_pathnames)
	{
		if (const auto lookup_iter = search_path.impl_archive_lookup.find(vfs_pathname);
			lookup_iter != search_path.impl_archive_lookup.cend())
		{
			if (ArchiveFileEntryStreamUPtr entry_stream = search_path.impl_archive->open_entry_stream(lookup_iter->second);
				entry_stream != nullptr)
				return std::make_unique<VfsImplArchiveInputStream>(std::move(entry_stream), vfs_pathname_index);
		}
		++vfs_pathname_index;
	}
	return nullptr;
}

VfsInputStreamUPtr VfsImpl::open_input_stream_file(const VfsSearchPathImpl& search_path, std::span<std::string_view> vfs_pathnames)
{
	const std::size_t vfs_pathname_max_size = std::ranges::max_element(vfs_pathnames, {}, &std::string_view::size)->size();
	std::string pathname{};
	pathname.reserve(search_path.impl_path.size() + 1 + vfs_pathname_max_size);
	int vfs_pathname_index = 0;
	for (const auto& vfs_pathname : vfs_pathnames)
	{
		pathname = search_path.impl_path;
		fs_utils::append_path_inplace(pathname, vfs_pathname);
		fs_utils::normalize_separators_inplace(pathname);
		if (sys::File file{pathname.data(), sys::FileMode::read};
			file.is_open())
			return std::make_unique<VfsImplFileInputStream>(std::move(file), vfs_pathname_index);
		++vfs_pathname_index;
	}
	return nullptr;
}

} // namespace

// ======================================

VfsUPtr make_vfs()
{
	return std::make_unique<VfsImpl>();
}

} // namespace bstone
