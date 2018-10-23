//
// A sprite.
//


#include "bstone_sprite.h"
#include <cstdint>
#include <memory>
#include <utility>
#include "bstone_endian.h"


namespace bstone
{


Sprite::Sprite() :
        left_{},
        right_{},
        top_{},
        bottom_{},
        image_{}
{
}

Sprite::Sprite(
    const Sprite& that) :
        left_{ that.left_ },
        right_{ that.right_ },
        top_{ that.top_ },
        bottom_{ that.bottom_ },
        image_{ that.image_ }
{
}

Sprite::Sprite(
    Sprite&& that) :
        left_{ std::move(that.left_) },
        right_{ std::move(that.right_) },
        top_{ std::move(that.top_) },
        bottom_{ std::move(that.bottom_) },
        image_{ std::move(that.image_) }
{
}

Sprite& Sprite::operator=(
    Sprite that)
{
    swap(*this, that);
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
        throw "No raw data.";
    }

    const auto values_16 = static_cast<const uint16_t*>(raw_data);
    const auto values_8 = static_cast<const uint8_t*>(raw_data);

	const auto& endian = bstone::Endian{};

    const int left = endian.little(values_16[0]);
    const int right = endian.little(values_16[1]);

    if (left > right || left >= side || right >= side)
    {
        throw "Invalid edge values.";
    }

    const auto commands_offsets = &values_16[2];


    // Find out vertical bounds.
    //
    int top = -1;
    int bottom = -1;

    for (int i = left; i <= right; ++i)
    {
        const int commands_offset = endian.little(commands_offsets[i - left]);
        const auto commands = reinterpret_cast<const uint16_t*>(&values_8[commands_offset]);

        int iCommand = 0;

        int end = endian.little(commands[iCommand++]) / 2;

        while (end != 0)
        {
            const int pixels_offset = endian.little(commands[iCommand++]);
            const int start = endian.little(commands[iCommand++]) / 2;

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
        const int commands_offset = endian.little(commands_offsets[i]);
        const auto commands = reinterpret_cast<const uint16_t*>(&values_8[commands_offset]);

        int iCommand = 0;

        int end = endian.little(commands[iCommand++]) / 2;

        while (end != 0)
        {
            int pixels_offset = endian.little(commands[iCommand++]);
            const int start = endian.little(commands[iCommand++]) / 2;
            const auto count = end - start;

            pixels_offset += start;
            pixels_offset %= 0x10000;

            const auto src_pixels = &values_8[pixels_offset];
            auto dst_pixels = &image_[(i * height) + (start - top)];

            std::uninitialized_copy_n(
                src_pixels,
                count,
                dst_pixels);

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
    Image{}.swap(image_);
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

void Sprite::swap(
    Sprite& a,
    Sprite& b)
{
    std::swap(a.left_, b.left_);
    std::swap(a.right_, b.right_);
    std::swap(a.top_, b.top_);
    std::swap(a.bottom_, b.bottom_);
    std::swap(a.image_, b.image_);
}


} // bstone
