/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#include "bstone_image_encoder.h"

#include <string>

#include "bstone_exception.h"
#include "bstone_stb_image_encoder.h"


namespace bstone
{


namespace
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class ImageEncoderException :
	public Exception
{
public:
	explicit ImageEncoderException(
		const char* message) noexcept
		:
		Exception{"IMAGE_ENCODER", message}
	{
	}
}; // ImageEncoderException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


[[noreturn]]
void fail(
	const char* message)
{
	throw ImageEncoderException{message};
}

[[noreturn]]
void fail_nested(
	const char* message)
{
	std::throw_with_nested(ImageEncoderException{message});
}


} // namespace


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

ImageEncodeUPtr make_image_encoder(
	ImageEncoderType image_encoder_type)
try
{
	switch (image_encoder_type)
	{
		case ImageEncoderType::png:
			return std::make_unique<StbImageEncoder>();

		default:
			fail("Unsupported image encoder type.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
