/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#include "bstone_image_decoder.h"

#include <string>

#include "bstone_exception.h"
#include "bstone_bmp_image_decoder.h"
#include "bstone_stb_image_decoder.h"


namespace bstone
{

ImageDecoder::ImageDecoder() noexcept = default;

ImageDecoder::~ImageDecoder() = default;

// ==========================================================================

ImageDecodeUPtr make_image_decoder(
	ImageDecoderType image_decoder_type)
try {
	switch (image_decoder_type)
	{
		case ImageDecoderType::bmp:
			return std::make_unique<BmpImageDecoder>();

		case ImageDecoderType::png:
			return std::make_unique<StbImageDecoder>();

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported image decoder type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // bstone
