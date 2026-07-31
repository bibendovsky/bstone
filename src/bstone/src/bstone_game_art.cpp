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

#include "bstone_fs_utils.h"
#include "bstone_image_decoder.h"
#include "bstone_sys_file.h"

namespace bstone {

namespace {

constexpr int max_art_file_size = 8 * 1024 * 1024;

// Where a store leaves the artwork, relative to the application holding the
// game. The names differ per store and per era, so try them in turn.
constexpr const char* art_file_names[] =
{
	"Contents/Resources/app.icns", // GOG.
	"Contents/Resources/Icon.icns", // Steam.
	"Contents/Resources/package.icns",
	"Contents/Resources/StandaloneLogo.png",
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

std::uint32_t read_u32_be(const unsigned char* bytes)
{
	return
		(static_cast<std::uint32_t>(bytes[0]) << 24) |
		(static_cast<std::uint32_t>(bytes[1]) << 16) |
		(static_cast<std::uint32_t>(bytes[2]) << 8) |
		static_cast<std::uint32_t>(bytes[3]);
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

	width = static_cast<int>(read_u32_be(&bytes[ihdr_offset]));
	height = static_cast<int>(read_u32_be(&bytes[ihdr_offset + 4]));
	return width > 0 && height > 0;
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
		const std::uint32_t chunk_size = read_u32_be(&icns[offset + 4]);

		if (chunk_size < chunk_header_size || offset + chunk_size > icns.size())
		{
			break;
		}

		const std::size_t data_offset = offset + chunk_header_size;
		const std::size_t data_size = chunk_size - chunk_header_size;
		int width = 0;
		int height = 0;

		if (read_png_size(&icns[data_offset], data_size, width, height))
		{
			// Prefer the smallest that still covers the size wanted; failing
			// that - every one is too small - the biggest there is.
			const bool is_better = best_width == 0 ||
				(best_width < desired_size ? width > best_width :
					(width >= desired_size && width < best_width));

			if (is_better)
			{
				best_offset = data_offset;
				best_size = data_size;
				best_width = width;
			}
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

		const std::size_t separator_pos = path.find_last_of("/\\");

		if (separator_pos == std::string::npos || separator_pos == 0)
		{
			break;
		}

		path.resize(separator_pos);
	}

	return false;
}

// Averages whole blocks of the source into each pixel, so shrinking keeps
// what is there instead of sampling a few pixels and missing the rest.
GameArt downscale(const GameArt& src, int size)
{
	auto dst = GameArt{};
	dst.width = size;
	dst.height = size;
	dst.pixels.resize(static_cast<std::size_t>(size) * size);

	for (int y = 0; y < size; ++y)
	{
		const int y0 = y * src.height / size;
		const int y1 = std::max(y0 + 1, (y + 1) * src.height / size);

		for (int x = 0; x < size; ++x)
		{
			const int x0 = x * src.width / size;
			const int x1 = std::max(x0 + 1, (x + 1) * src.width / size);
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

			Rgba8& out = dst.pixels[static_cast<std::size_t>(y) * size + x];

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

	if (image_decoder == nullptr)
	{
		return art;
	}

	try
	{
		image_decoder->decode(png.data(), static_cast<int>(png.size()), art.width, art.height, art.pixels);
	}
	catch (...)
	{
		return GameArt{};
	}

	if (art.width > desired_size && art.width == art.height)
	{
		art = downscale(art, desired_size);
	}

	return art;
}

} // namespace bstone
