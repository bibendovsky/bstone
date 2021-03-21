/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// A sprite.
//


#include "bstone_sprite.h"

#include <stdexcept>
#include <memory>

#include "bstone_endian.h"


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
	Sprite&& rhs)
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
		throw std::runtime_error{"No raw data."};
	}

	const auto values_16 = static_cast<const std::uint16_t*>(raw_data);
	const auto values_8 = static_cast<const std::uint8_t*>(raw_data);

	const int left = bstone::Endian::little(values_16[0]);
	const int right = bstone::Endian::little(values_16[1]);

	if (left > right || left >= dimension || right >= dimension)
	{
		throw std::runtime_error{"Invalid edge values."};
	}

	const auto commands_offsets = &values_16[2];


	// Find out vertical bounds.
	//
	auto top = -1;
	auto bottom = -1;

	for (int i = left; i <= right; ++i)
	{
		const auto commands_offset = bstone::Endian::little(commands_offsets[i - left]);
		const auto commands = reinterpret_cast<const std::uint16_t*>(&values_8[commands_offset]);

		auto i_command = 0;

		auto end = bstone::Endian::little(commands[i_command++]) / 2;

		while (end != 0)
		{
			i_command += 1; // pixels offset

			const auto start = bstone::Endian::little(commands[i_command++]) / 2;

			if (top < 0 || start < top)
			{
				top = start;
			}

			if (bottom < 0 || end > bottom)
			{
				bottom = end;
			}

			end = bstone::Endian::little(commands[i_command++]) / 2;
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
		const auto commands_offset = static_cast<int>(bstone::Endian::little(commands_offsets[i]));
		const auto commands = reinterpret_cast<const std::uint16_t*>(&values_8[commands_offset]);

		auto i_command = 0;

		auto end = bstone::Endian::little(commands[i_command++]) / 2;

		while (end != 0)
		{
			auto pixels_offset = static_cast<int>(bstone::Endian::little(commands[i_command++]));
			const auto start = static_cast<int>(bstone::Endian::little(commands[i_command++]) / 2);
			const auto count = end - start;

			pixels_offset += start;
			pixels_offset %= 0x10000;

			const auto src_pixels = &values_8[pixels_offset];
			auto dst_pixels = &image_[(i * height) + (start - top)];

			std::uninitialized_copy_n(src_pixels, count, dst_pixels);

			end = bstone::Endian::little(commands[i_command++]) / 2;
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
	image_ = {};
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
	return (right_ - left_) + 1;
}

int Sprite::get_height() const
{
	return (bottom_ - top_) + 1;
}

const std::int16_t* Sprite::get_column(
	const int index) const
{
	return &image_[index * get_height()];
}

const std::int16_t* Sprite::get_data() const
{
	return image_.data();
}


} // bstone
