/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#ifndef BSTONE_IMAGE_DECODER_INCLUDED
#define BSTONE_IMAGE_DECODER_INCLUDED


#include <memory>

#include "bstone_rgb8.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ImageDecoder
{
public:
	ImageDecoder() noexcept;

	virtual ~ImageDecoder();


	virtual void decode(
		const void* src_data,
		int src_data_size,
		int& dst_width,
		int& dst_height,
		Rgba8Buffer& dst_buffer) = 0;
}; // ImageDecoder

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

enum class ImageDecoderType
{
	none,
	bmp,
	png,
}; // ImageDecoderType

using ImageDecodeUPtr = std::unique_ptr<ImageDecoder>;

ImageDecodeUPtr make_image_decoder(
	ImageDecoderType image_decoder_type);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_IMAGE_DECODER_INCLUDED
