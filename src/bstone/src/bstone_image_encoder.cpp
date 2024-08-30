/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#include "bstone_image_encoder.h"

#include <string>

#include "bstone_exception.h"
#include "bstone_stb_image_encoder.h"

namespace bstone {

ImageEncoder::ImageEncoder() noexcept = default;

ImageEncoder::~ImageEncoder() = default;

// ==========================================================================

ImageEncodeUPtr make_image_encoder(
	ImageEncoderType image_encoder_type)
try {
	switch (image_encoder_type)
	{
		case ImageEncoderType::png:
			return std::make_unique<StbImageEncoder>();

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported image encoder type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // bstone
