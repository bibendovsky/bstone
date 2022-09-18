/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#include "bstone_image_decoder.h"

#include <string>

#include "bstone_exception.h"
#include "bstone_bmp_image_decoder.h"
#include "bstone_stb_image_decoder.h"


namespace bstone
{


namespace
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ImageDecoderException :
	public Exception
{
public:
	explicit ImageDecoderException(
		const char* message) noexcept
		:
		Exception{"IMAGE_DECODER", message}
	{
	}
}; // ImageDecoderException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


[[noreturn]]
void fail(
	const char* message)
{
	throw ImageDecoderException{message};
}

[[noreturn]]
void fail_nested(
	const char* message)
{
	std::throw_with_nested(ImageDecoderException{message});
}


} // namespace


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

ImageDecodeUPtr make_image_decoder(
	ImageDecoderType image_decoder_type)
try
{
	switch (image_decoder_type)
	{
		case ImageDecoderType::bmp:
			return std::make_unique<BmpImageDecoder>();

		case ImageDecoderType::png:
			return std::make_unique<StbImageDecoder>();

		default:
			fail("Unsupported image decoder type.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
