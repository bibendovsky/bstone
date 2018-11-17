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

	const auto& endian = bstone::Endian{};

	const int left = endian.little(values_16[0]);
	const int right = endian.little(values_16[1]);

	if (left > right || left >= side || right >= side)
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
		const auto commands_offset = endian.little(commands_offsets[i - left]);
		const auto commands = reinterpret_cast<const std::uint16_t*>(&values_8[commands_offset]);

		auto iCommand = 0;

		auto end = endian.little(commands[iCommand++]) / 2;

		while (end != 0)
		{
			const auto pixels_offset = static_cast<int>(endian.little(commands[iCommand++]));
			const auto start = endian.little(commands[iCommand++]) / 2;

			if (top < 0 || start < top)
			{
				top = start;
			}

			if (bottom < 0 || end > bottom)
			{
				bottom = end;
			}

			end = endian.little(commands[iCommand++]) / 2;
		}
	}

	// Build the image.
	//

	const auto width = (right - left) + 1;
	const auto height = (bottom - top) + 1;
	const auto area = width * height;

	image_.resize(area, -1);

	for (int i = 0; i < width; ++i)
	{
		const auto commands_offset = static_cast<int>(endian.little(commands_offsets[i]));
		const auto commands = reinterpret_cast<const std::uint16_t*>(&values_8[commands_offset]);

		auto iCommand = 0;

		auto end = endian.little(commands[iCommand++]) / 2;

		while (end != 0)
		{
			auto pixels_offset = static_cast<int>(endian.little(commands[iCommand++]));
			const auto start = static_cast<int>(endian.little(commands[iCommand++]) / 2);
			const auto count = end - start;

			pixels_offset += start;
			pixels_offset %= 0x10000;

			const auto src_pixels = &values_8[pixels_offset];
			auto dst_pixels = &image_[(i * height) + (start - top)];

			std::uninitialized_copy_n(src_pixels, count, dst_pixels);

			end = endian.little(commands[iCommand++]) / 2;
		}
	}

	left_ = left;
	right_ = right;
	top_ = top;
	bottom_ = bottom;
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

const short* Sprite::get_column(
	const int index) const
{
	return &image_[index * get_height()];
}


} // bstone
