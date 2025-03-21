/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_level_extractor.h"

#include <utility>

#include "id_ca.h"

#include "bstone_file.h"
#include "bstone_fs.h"
#include "bstone_fs_utils.h"
#include "bstone_globals.h"

namespace bstone {

void LevelExtractor::extract_levels(const std::string& destination_dir)
{
	globals::logger->log_information();
	globals::logger->log_information("<<< ================");
	globals::logger->log_information("Extracting levels.");
	globals::logger->log_information(("Destination dir: \"" + destination_dir + "\"").c_str());

	const std::string normalized_destination_dir = fs_utils::normalize_path(destination_dir);
	fs::create_directories(normalized_destination_dir.c_str());

	std::uint16_t plane_buffer[MAPSIZE * MAPSIZE];

	auto tmp_file_path = std::string{};
	tmp_file_path.reserve(normalized_destination_dir.size() + 16);
	tmp_file_path = normalized_destination_dir;
	tmp_file_path = bstone::fs_utils::append_path(tmp_file_path, "_.temp");

	auto dst_file_path = std::string{};
	dst_file_path.reserve(normalized_destination_dir.size() + 16);

	const auto level_count = mapheaderseg.size();

	for (auto level_index = decltype(level_count){}; level_index < level_count; ++level_index)
	{
		CA_CacheMap(static_cast<int>(level_index));

		dst_file_path = normalized_destination_dir;
		dst_file_path = bstone::fs_utils::append_path(dst_file_path, "level_");
		dst_file_path += std::to_string(level_index);
		dst_file_path += ".flr";

		{
			File file(
				tmp_file_path.c_str(),
				bstone::file_flags_create | bstone::file_flags_truncate | bstone::file_flags_exclusive);

			if (!file.is_open())
			{
				std::string error_message;
				error_message.reserve(1024);
				error_message += "Could not create a file \"";
				error_message += tmp_file_path;
				error_message += "\".";
				BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
			}

			for (const MapSegment& plane : mapsegs)
			{
				int dst_index = 0;

				// Rotate clockwise by 90 degree.
				for (int x = 0; x < MAPSIZE; ++x)
				{
					for (int y = 0; y < MAPSIZE; ++y)
					{
						const int src_index = y * MAPSIZE + x;
						plane_buffer[dst_index++] = plane[src_index];
					}
				}

				if (!file.write_exactly(plane_buffer, 8192))
				{
					std::string error_message;
					error_message.reserve(1024);
					error_message += "Could not write into a file \"";
					error_message += tmp_file_path;
					error_message += "\".";
					BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
				}
			}
		}

		fs::remove_if_exists(dst_file_path.c_str());
		fs::rename(tmp_file_path.c_str(), dst_file_path.c_str());
	}

	globals::logger->log_information(("Extracted " + std::to_string(level_count) + " levels.").c_str());
	globals::logger->log_information(">>> ================");
}

} // namespace bstone
