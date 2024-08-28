/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_STB_IMAGE_DECODER_INCLUDED
#define BSTONE_STB_IMAGE_DECODER_INCLUDED

#include "bstone_image_decoder.h"

namespace bstone {

class StbImageDecoder : public ImageDecoder
{
public:
	~StbImageDecoder() override;

	void decode(
		const void* src_data,
		int src_data_size,
		int& dst_width,
		int& dst_height,
		Rgba8Buffer& dst_buffer) override;
};

} // namespace bstone

#endif // BSTONE_STB_IMAGE_DECODER_INCLUDED
