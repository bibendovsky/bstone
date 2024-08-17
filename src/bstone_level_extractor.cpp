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

	std::uint16_t plane_buffer[MAPSIZE * MAPSIZE];

	auto tmp_file_path = std::string{};
	tmp_file_path.reserve(destination_dir.size() + 16);
	tmp_file_path = destination_dir;
	tmp_file_path = bstone::fs_utils::append_path(tmp_file_path, "_.temp");

	auto dst_file_path = std::string{};
	dst_file_path.reserve(destination_dir.size() + 16);

	const auto level_count = mapheaderseg.size();

	for (auto level_index = decltype(level_count){}; level_index < level_count; ++level_index)
	{
		CA_CacheMap(static_cast<int>(level_index));

		dst_file_path = destination_dir;
		dst_file_path = bstone::fs_utils::append_path(dst_file_path, "level_");
		dst_file_path += std::to_string(level_index);
		dst_file_path += ".flr";

		{
			auto file = File{
				tmp_file_path.c_str(),
				bstone::FileOpenFlags::create | bstone::FileOpenFlags::truncate,
				bstone::FileShareMode::exclusive};

			for (const auto& plane : mapsegs)
			{
				auto dst_index = 0;

				// Rotate clockwise by 90 degree.
				for (auto x = 0; x < MAPSIZE; ++x)
				{
					for (auto y = 0; y < MAPSIZE; ++y)
					{
						const auto src_index = y * MAPSIZE + x;
						plane_buffer[dst_index++] = plane[src_index];
					}
				}

				file.write_exactly(plane_buffer, 8192);
			}
		}

		fs::remove_if_exists(dst_file_path.c_str());
		fs::rename(tmp_file_path.c_str(), dst_file_path.c_str());
	}

	globals::logger->log_information(("Extracted " + std::to_string(level_count) + " levels.").c_str());
	globals::logger->log_information(">>> ================");
}

} // namespace bstone
