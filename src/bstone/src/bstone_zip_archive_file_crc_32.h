/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_ZIP_ARCHIVE_FILE_CRC_32_ICNLUDED
#define BSTONE_ZIP_ARCHIVE_FILE_CRC_32_ICNLUDED

namespace bstone {

class ZipArchiveFileCrc32
{
public:
	unsigned int get_value() const;

	bool is_finished() const;
	void reset();
	void update(const void* buffer, int count);
	void finish();

private:
	bool is_finished_;
	unsigned int value_;
};

} // namespace bstone

#endif // BSTONE_ZIP_ARCHIVE_FILE_CRC_32_ICNLUDED
