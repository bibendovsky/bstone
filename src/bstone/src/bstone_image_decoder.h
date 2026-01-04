/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Image decoder

#ifndef BSTONE_IMAGE_DECODER_INCLUDED
#define BSTONE_IMAGE_DECODER_INCLUDED

#include "bstone_rgb8.h"
#include <memory>

namespace bstone {

enum class ImageDecoderType
{
	none,
	bmp,
	png,
};

class ImageDecoder
{
public:
	ImageDecoder() = default;
	virtual ~ImageDecoder() = default;

	virtual void decode(
		const void* src_data,
		int src_data_size,
		int& dst_width,
		int& dst_height,
		Rgba8Buffer& dst_buffer) = 0;
};

// ======================================

using ImageDecoderUPtr = std::unique_ptr<ImageDecoder>;

ImageDecoderUPtr make_image_decoder(ImageDecoderType image_decoder_type);

} // namespace bstone

#endif // BSTONE_IMAGE_DECODER_INCLUDED
