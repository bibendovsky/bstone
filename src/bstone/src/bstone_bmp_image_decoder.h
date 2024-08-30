/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_BMP_IMAGE_DECODER_INCLUDED
#define BSTONE_BMP_IMAGE_DECODER_INCLUDED

#include "bstone_image_decoder.h"

namespace bstone {

class BmpImageDecoder : public ImageDecoder
{
public:
	~BmpImageDecoder() override;

	void decode(
		const void* src_data,
		int src_data_size,
		int& dst_width,
		int& dst_height,
		Rgba8Buffer& dst_buffer) override;
};

} // namespace bstone

#endif // BSTONE_BMP_IMAGE_DECODER_INCLUDED
