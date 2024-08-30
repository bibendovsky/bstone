/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// A sprite.
//


#include "bstone_sprite.h"

#include <algorithm>
#include <memory>

#include "bstone_endian.h"
#include "bstone_exception.h"


namespace bstone
{


Sprite::Sprite()
	:
	left_{},
	right_{},
	top_{},
	bottom_{},
	image_{}
{
}

Sprite::Sprite(
	const Sprite& rhs)
	:
	left_{rhs.left_},
	right_{rhs.right_},
	top_{rhs.top_},
	bottom_{rhs.bottom_},
	image_{rhs.image_}
{
}

Sprite::Sprite(
	Sprite&& rhs) noexcept
	:
	left_{std::move(rhs.left_)},
	right_{std::move(rhs.right_)},
	top_{std::move(rhs.top_)},
	bottom_{std::move(rhs.bottom_)},
	image_{std::move(rhs.image_)}
{
}

Sprite& Sprite::operator=(
	const Sprite& rhs)
{
	if (std::addressof(rhs) != this)
	{
		left_ = rhs.left_;
		right_ = rhs.right_;
		top_ = rhs.top_;
		bottom_ = rhs.bottom_;
		image_ = rhs.image_;
	}

	return *this;
}

Sprite::~Sprite()
{
}

void Sprite::initialize(
	const void* raw_data)
{
	uninitialize();

	if (!raw_data)
	{
		BSTONE_THROW_STATIC_SOURCE("No raw data.");
	}

	const auto values_16 = static_cast<const std::uint16_t*>(raw_data);
	const auto values_8 = static_cast<const std::uint8_t*>(raw_data);

	const int left = bstone::endian::to_little(values_16[0]);
	const int right = bstone::endian::to_little(values_16[1]);

	if (left > right || left >= dimension || right >= dimension)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid edge values.");
	}

	const auto commands_offsets = &values_16[2];


	// Find out vertical bounds.
	//
	auto top = -1;
	auto bottom = -1;

	for (int i = left; i <= right; ++i)
	{
		const auto commands_offset = bstone::endian::to_little(commands_offsets[i - left]);
		const auto commands = reinterpret_cast<const std::uint16_t*>(&values_8[commands_offset]);

		auto i_command = 0;

		auto end = bstone::endian::to_little(commands[i_command++]) / 2;

		while (end != 0)
		{
			i_command += 1; // pixels offset

			const auto start = bstone::endian::to_little(commands[i_command++]) / 2;

			if (top < 0 || start < top)
			{
				top = start;
			}

			if (bottom < 0 || end > bottom)
			{
				bottom = end;
			}

			end = bstone::endian::to_little(commands[i_command++]) / 2;
		}
	}

	// Build the image.
	//

	const auto width = (right - left) + 1;
	const auto height = bottom - top;
	const auto area = width * height;

	image_.resize(area, -1);

	for (int i = 0; i < width; ++i)
	{
		const auto commands_offset = static_cast<int>(bstone::endian::to_little(commands_offsets[i]));
		const auto commands = reinterpret_cast<const std::uint16_t*>(&values_8[commands_offset]);

		auto i_command = 0;

		auto end = bstone::endian::to_little(commands[i_command++]) / 2;

		while (end != 0)
		{
			auto pixels_offset = static_cast<int>(bstone::endian::to_little(commands[i_command++]));
			const auto start = static_cast<int>(bstone::endian::to_little(commands[i_command++]) / 2);
			const auto count = end - start;

			pixels_offset += start;
			pixels_offset %= 0x10000;

			const auto src_pixels = &values_8[pixels_offset];
			auto dst_pixels = &image_[(i * height) + (start - top)];

			std::copy_n(src_pixels, count, dst_pixels);

			end = bstone::endian::to_little(commands[i_command++]) / 2;
		}
	}

	left_ = left;
	right_ = right;
	top_ = top;
	bottom_ = bottom - 1;
}

void Sprite::uninitialize() noexcept
{
	left_ = 0;
	right_ = 0;
	image_ = {};
}

bool Sprite::is_initialized() const noexcept
{
	return !image_.empty();
}

int Sprite::get_left() const noexcept
{
	return left_;
}

int Sprite::get_right() const noexcept
{
	return right_;
}

int Sprite::get_top() const noexcept
{
	return top_;
}

int Sprite::get_bottom() const noexcept
{
	return bottom_;
}

int Sprite::get_width() const noexcept
{
	return (right_ - left_) + 1;
}

int Sprite::get_height() const noexcept
{
	return (bottom_ - top_) + 1;
}

const std::int16_t* Sprite::get_column(int index) const noexcept
{
	return &image_[index * get_height()];
}

const std::int16_t* Sprite::get_data() const noexcept
{
	return image_.data();
}


} // bstone
