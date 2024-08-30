/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#ifndef BSTONE_IMAGE_ENCODER_INCLUDED
#define BSTONE_IMAGE_ENCODER_INCLUDED


#include <cstdint>

#include <memory>


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ImageEncoder
{
public:
	ImageEncoder() noexcept;

	virtual ~ImageEncoder();


	virtual void encode_24(
		const std::uint8_t* src_buffer,
		int src_width,
		int src_height,
		std::uint8_t* dst_buffer,
		int dst_buffer_max_size,
		int& dst_size) = 0;
}; // ImageEncoder

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

enum class ImageEncoderType
{
	none,
	png,
}; // ImageEncoderType

using ImageEncodeUPtr = std::unique_ptr<ImageEncoder>;

ImageEncodeUPtr make_image_encoder(
	ImageEncoderType image_encoder_type);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_IMAGE_ENCODER_INCLUDED
