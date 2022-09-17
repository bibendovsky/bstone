/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_STB_IMAGE_ENCODER_INCLUDED
#define BSTONE_STB_IMAGE_ENCODER_INCLUDED


#include "bstone_image_encoder.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class StbImageEncoder :
	public ImageEncoder
{
public:
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


	static void stb_write_func_proxy(
		void* context,
		void* data,
		int size) noexcept;

	void stb_write_func(
		void* data,
		int size) noexcept;
}; // StbImageEncoder

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_STB_IMAGE_ENCODER_INCLUDED
