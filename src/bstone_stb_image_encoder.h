/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_STB_IMAGE_ENCODER_INCLUDED
#define BSTONE_STB_IMAGE_ENCODER_INCLUDED

#include "bstone_image_encoder.h"

namespace bstone {

class StbImageEncoder : public ImageEncoder
{
public:
	~StbImageEncoder() override;

	void encode_24(
		const std::uint8_t* src_buffer,
		int src_width,
		int src_height,
		std::uint8_t* dst_buffer,
		int max_dst_buffer_size,
		int& dst_size) override;

private:
	std::uint8_t* dst_buffer_{};
	int size_{};
	int max_size_{};

private:
	static void stb_write_func_proxy(void* context, void* data, int size) noexcept;
	void stb_write_func(void* data, int size) noexcept;
};

} // namespace bstone

#endif // BSTONE_STB_IMAGE_ENCODER_INCLUDED
