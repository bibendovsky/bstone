/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// A sprite.

#include "bstone_sprite.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_memory_binary_reader.h"
#include <algorithm>
#include <memory>

namespace bstone {

void Sprite::initialize(const void* raw_data, int raw_data_size)
{
	uninitialize();
	const std::uint8_t* const bytes = static_cast<const std::uint8_t*>(raw_data);
	const int left = endian::read_u16_le(bytes);
	const int right = endian::read_u16_le(bytes + 2);
	if (left > right || left >= dimension || right >= dimension)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid edge values.");
	}
	const int width = right - left + 1;
	// Find out vertical bounds.
	int top = -1;
	int bottom = -1;
	for (int i = 0; i < width; ++i)
	{
		const int commands_offset = endian::read_u16_le(bytes + 4 + i * 2);
		MemoryBinaryReader cmd_reader{bytes + commands_offset, raw_data_size - commands_offset};
		if (!cmd_reader.can_read_x16())
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid data.");
		}
		for (int end = cmd_reader.read_u16_le() / 2; end != 0; end = cmd_reader.read_u16_le() / 2)
		{
			if (!cmd_reader.can_read_n(6))
			{
				BSTONE_THROW_STATIC_SOURCE("Invalid data.");
			}
			int pixels_offset = cmd_reader.read_u16_le();
			const int start = cmd_reader.read_u16_le() / 2;
			const int count = end - start;
			pixels_offset += start;
			pixels_offset %= 0x10000;
			if (pixels_offset + count > raw_data_size)
			{
				BSTONE_THROW_STATIC_SOURCE("Invalid data.");
			}
			if (top < 0 || start < top)
			{
				top = start;
			}
			if (bottom < 0 || end > bottom)
			{
				bottom = end;
			}
		}
	}
	// Build the image.
	const int height = bottom - top;
	const int area = width * height;
	image_.resize(area, -1);
	for (int i = 0; i < width; ++i)
	{
		const int commands_offset = endian::read_u16_le(bytes + 4 + i * 2);
		MemoryBinaryReader cmd_reader{bytes + commands_offset, raw_data_size - commands_offset};
		for (int end = cmd_reader.read_u16_le() / 2; end != 0; end = cmd_reader.read_u16_le() / 2)
		{
			int pixels_offset = cmd_reader.read_u16_le();
			const int start = cmd_reader.read_u16_le() / 2;
			const int count = end - start;
			pixels_offset += start;
			pixels_offset %= 0x10000;
			const std::uint8_t* const src_pixels = bytes + pixels_offset;
			std::int16_t* const dst_pixels = image_.data() + i * height + start - top;
			std::copy_n(src_pixels, count, dst_pixels);
		}
	}
	left_ = left;
	right_ = right;
	top_ = top;
	bottom_ = bottom - 1;
}

void Sprite::uninitialize()
{
	left_ = 0;
	right_ = 0;
	top_ = 0;
	bottom_ = 0;
	image_.clear();
}

bool Sprite::is_initialized() const
{
	return !image_.empty();
}

int Sprite::get_left() const
{
	return left_;
}

int Sprite::get_right() const
{
	return right_;
}

int Sprite::get_top() const
{
	return top_;
}

int Sprite::get_bottom() const
{
	return bottom_;
}

int Sprite::get_width() const
{
	return right_ - left_ + 1;
}

int Sprite::get_height() const
{
	return bottom_ - top_ + 1;
}

const std::int16_t* Sprite::get_column(int index) const
{
	return get_data() + index * get_height();
}

const std::int16_t* Sprite::get_data() const
{
	return image_.data();
}

} // namespace bstone
