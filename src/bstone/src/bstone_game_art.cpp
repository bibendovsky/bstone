/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Artwork shipped alongside a game the user provided.

#include "bstone_game_art.h"

#include <cstdint>
#include <cstring>

#include <algorithm>

#include "bstone_ascii.h"
#include "bstone_content_path.h"
#include "bstone_endian.h"
#include "bstone_fs_utils.h"
#include "bstone_image_decoder.h"
#include "bstone_sys_file.h"
#include "bstone_sys_fs.h"

namespace bstone {

namespace {

constexpr int max_art_file_size = 8 * 1024 * 1024;

// Where a store leaves the artwork, relative to the application holding the
// game. The names differ per store and per era, so try them in turn.
constexpr const char* art_file_names[] =
{
	"Contents/Resources/app.icns", // GOG, macOS.
	"Contents/Resources/Icon.icns", // Steam, macOS.
	"Contents/Resources/package.icns",
	"Contents/Resources/StandaloneLogo.png",
	"support/icon.png", // GOG, Linux.
	"icon.png",
	"cover.png",
};

using Bytes = std::vector<unsigned char>;

bool read_file(const std::string& path, Bytes& bytes)
{
	auto file = sys::File{};

	if (!file.open(path.c_str(), sys::FileMode::read))
	{
		return false;
	}

	const std::int64_t size = file.get_size();

	if (size <= 0 || size > max_art_file_size)
	{
		return false;
	}

	bytes.resize(static_cast<std::size_t>(size));
	return file.read_exactly(bytes.data(), static_cast<int>(size));
}

bool is_png(const unsigned char* bytes, std::size_t size)
{
	static constexpr unsigned char png_signature[] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
	return size >= sizeof(png_signature) && std::memcmp(bytes, png_signature, sizeof(png_signature)) == 0;
}

// A PNG says its size in the header, which is enough to choose between the
// sizes an icon file holds without decoding any of them.
bool read_png_size(const unsigned char* bytes, std::size_t size, int& width, int& height)
{
	constexpr std::size_t ihdr_offset = 16;

	if (size < ihdr_offset + 8 || !is_png(bytes, size))
	{
		return false;
	}

	width = static_cast<int>(endian::read_u32_be(&bytes[ihdr_offset]));
	height = static_cast<int>(endian::read_u32_be(&bytes[ihdr_offset + 4]));
	return width > 0 && height > 0;
}

// Prefer the smallest that still covers the size wanted; failing that - every
// one is too small - the biggest there is.
bool is_better_size(int width, int best_width, int desired_size)
{
	return best_width == 0 ||
		(best_width < desired_size ? width > best_width :
			(width >= desired_size && width < best_width));
}

// An icns file is a header and then a run of typed chunks. It holds the same
// icon at many sizes - 32 through 1024 - so take the smallest one that is
// still at least as big as it will be drawn. Taking the largest and letting
// it be squeezed down throws away most of its pixels and looks it.
bool extract_png_from_icns(const Bytes& icns, int desired_size, Bytes& png)
{
	constexpr std::size_t header_size = 8;
	constexpr std::size_t chunk_header_size = 8;

	if (icns.size() < header_size || std::memcmp(icns.data(), "icns", 4) != 0)
	{
		return false;
	}

	std::size_t offset = header_size;
	std::size_t best_offset = 0;
	std::size_t best_size = 0;
	int best_width = 0;

	while (offset + chunk_header_size <= icns.size())
	{
		const std::uint32_t chunk_size = endian::read_u32_be(&icns[offset + 4]);

		if (chunk_size < chunk_header_size || offset + chunk_size > icns.size())
		{
			break;
		}

		const std::size_t data_offset = offset + chunk_header_size;
		const std::size_t data_size = chunk_size - chunk_header_size;
		int width = 0;
		int height = 0;

		if (read_png_size(&icns[data_offset], data_size, width, height) &&
			is_better_size(width, best_width, desired_size))
		{
			best_offset = data_offset;
			best_size = data_size;
			best_width = width;
		}

		offset += chunk_size;
	}

	if (best_size == 0)
	{
		return false;
	}

	png.assign(icns.cbegin() + static_cast<std::ptrdiff_t>(best_offset),
		icns.cbegin() + static_cast<std::ptrdiff_t>(best_offset + best_size));

	return true;
}

// An icon file is a directory of images, held at the same range of sizes as an
// icns and chosen between the same way. An entry may also be a bitmap rather
// than a PNG, which is not read: the icons this looks for are PNG throughout.
bool extract_png_from_ico(const Bytes& ico, int desired_size, Bytes& png)
{
	constexpr std::size_t header_size = 6;
	constexpr std::size_t entry_size = 16;
	constexpr std::uint16_t icon_type = 1;

	if (ico.size() < header_size ||
		endian::read_u16_le(&ico[0]) != 0 ||
		endian::read_u16_le(&ico[2]) != icon_type)
	{
		return false;
	}

	const std::size_t count = endian::read_u16_le(&ico[4]);
	std::size_t best_offset = 0;
	std::size_t best_size = 0;
	int best_width = 0;

	for (std::size_t i = 0; i < count; ++i)
	{
		const std::size_t entry_offset = header_size + i * entry_size;

		if (entry_offset + entry_size > ico.size())
		{
			break;
		}

		const unsigned char* const entry = &ico[entry_offset];
		const std::size_t data_size = endian::read_u32_le(&entry[8]);
		const std::size_t data_offset = endian::read_u32_le(&entry[12]);

		if (data_size == 0 || data_offset > ico.size() || data_size > ico.size() - data_offset)
		{
			continue;
		}

		int width = 0;
		int height = 0;

		// The size the image gives is the one to trust: the directory has only
		// a byte for it, too narrow to say anything above 256.
		if (read_png_size(&ico[data_offset], data_size, width, height) &&
			is_better_size(width, best_width, desired_size))
		{
			best_offset = data_offset;
			best_size = data_size;
			best_width = width;
		}
	}

	if (best_size == 0)
	{
		return false;
	}

	png.assign(ico.cbegin() + static_cast<std::ptrdiff_t>(best_offset),
		ico.cbegin() + static_cast<std::ptrdiff_t>(best_offset + best_size));

	return true;
}

// GOG names the icon after the game's id, so it has to be looked for rather
// than named outright.
bool find_gog_icon_path(const std::string& directory_path, std::string& icon_path)
{
	struct Search
	{
		const std::string* directory_path;
		std::string* icon_path;
		bool is_found;
	};

	auto search = Search{&directory_path, &icon_path, false};
	sys::enumerate_directory(
		directory_path.c_str(),
		[](void* user_data, const char*, const char* file_name) -> sys::EnumDirCallbackResult
		{
			auto& state = *static_cast<Search*>(user_data);
			const std::string name = file_name;
			constexpr const char* prefix = "goggame-";
			constexpr const char* suffix = ".ico";
			constexpr std::size_t prefix_size = 8;
			constexpr std::size_t suffix_size = 4;

			// Something has to stand between the two for there to be an id.
			if (name.size() <= prefix_size + suffix_size)
			{
				return sys::EnumDirCallbackResult::resume;
			}

			const char* const extension = name.c_str() + name.size() - suffix_size;

			if (!ascii::starts_with_ignoring_case(name.c_str(), prefix) ||
				!ascii::starts_with_ignoring_case(extension, suffix))
			{
				return sys::EnumDirCallbackResult::resume;
			}

			*state.icon_path = fs_utils::append_path(*state.directory_path, name);
			state.is_found = true;
			return sys::EnumDirCallbackResult::success;
		},
		&search);

	return search.is_found;
}

// The game's files sit deep inside the application that holds them, so walk
// back up looking for the artwork at each step.
bool find_art_bytes(const std::string& game_path, int desired_size, Bytes& png)
{
	std::string path = game_path;

	while (!path.empty())
	{
		for (const char* art_file_name : art_file_names)
		{
			auto bytes = Bytes{};

			if (!read_file(fs_utils::append_path(path, art_file_name), bytes))
			{
				continue;
			}

			if (is_png(bytes.data(), bytes.size()))
			{
				png = std::move(bytes);
				return true;
			}

			if (extract_png_from_icns(bytes, desired_size, png))
			{
				return true;
			}
		}

		// What GOG leaves on Windows and Linux, where there is no application
		// bundle carrying an icon.
		std::string gog_icon_path;

		if (find_gog_icon_path(path, gog_icon_path))
		{
			auto bytes = Bytes{};

			if (read_file(gog_icon_path, bytes) &&
				extract_png_from_ico(bytes, desired_size, png))
			{
				return true;
			}
		}

		const std::size_t separator_pos = path.find_last_of("/\\");

		if (separator_pos == std::string::npos || separator_pos == 0)
		{
			break;
		}

		path.resize(separator_pos);
	}

	// Nothing beside the game. An application holding one carries an icon, but
	// a plain install has none, so fall back to what Steam downloaded and kept
	// with its own data - the only artwork such an installation has.
	const std::string steam_art_path = make_content_art_path(game_path);

	if (!steam_art_path.empty() && read_file(steam_art_path, png) &&
		is_png(png.data(), png.size()))
	{
		return true;
	}

	png.clear();
	return false;
}

// Averages whole blocks of the source into each pixel, so shrinking keeps
// what is there instead of sampling a few pixels and missing the rest.
GameArt downscale(const GameArt& src, int dst_width, int dst_height)
{
	auto dst = GameArt{};
	dst.width = dst_width;
	dst.height = dst_height;
	dst.pixels.resize(static_cast<std::size_t>(dst_width) * dst_height);

	for (int y = 0; y < dst_height; ++y)
	{
		const int y0 = y * src.height / dst_height;
		const int y1 = std::max(y0 + 1, (y + 1) * src.height / dst_height);

		for (int x = 0; x < dst_width; ++x)
		{
			const int x0 = x * src.width / dst_width;
			const int x1 = std::max(x0 + 1, (x + 1) * src.width / dst_width);
			int r = 0;
			int g = 0;
			int b = 0;
			int a = 0;
			int count = 0;

			for (int sy = y0; sy < y1; ++sy)
			{
				for (int sx = x0; sx < x1; ++sx)
				{
					const Rgba8& pixel = src.pixels[static_cast<std::size_t>(sy) * src.width + sx];
					// Weight colour by coverage, or transparent pixels drag
					// the edges towards whatever colour they happen to hold.
					r += pixel.r_ * pixel.a_;
					g += pixel.g_ * pixel.a_;
					b += pixel.b_ * pixel.a_;
					a += pixel.a_;
					++count;
				}
			}

			Rgba8& out = dst.pixels[static_cast<std::size_t>(y) * dst_width + x];

			if (a > 0)
			{
				out.r_ = static_cast<std::uint8_t>(r / a);
				out.g_ = static_cast<std::uint8_t>(g / a);
				out.b_ = static_cast<std::uint8_t>(b / a);
				out.a_ = static_cast<std::uint8_t>(a / count);
			}
			else
			{
				out = Rgba8{};
			}
		}
	}

	return dst;
}

} // namespace

GameArt find_game_art(const std::string& game_path, int desired_size)
{
	auto art = GameArt{};
	auto png = Bytes{};

	if (desired_size <= 0 || !find_art_bytes(game_path, desired_size, png))
	{
		return art;
	}

	const ImageDecoderUPtr image_decoder = make_image_decoder(ImageDecoderType::png);

	try
	{
		image_decoder->decode(png.data(), static_cast<int>(png.size()), art.width, art.height, art.pixels);
	}
	catch (...)
	{
		return GameArt{};
	}

	// A store icon is square, but a logo is not, so shrink by the longer side
	// and let the shorter one fall where it does.
	const int longest_side = std::max(art.width, art.height);

	if (longest_side > desired_size)
	{
		art = downscale(
			art,
			std::max(1, art.width * desired_size / longest_side),
			std::max(1, art.height * desired_size / longest_side));
	}

	return art;
}

} // namespace bstone
