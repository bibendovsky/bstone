/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// Audio sample converter.
//

#ifndef BSTONE_AUDIO_SAMPLE_CONVERTER_INCLUDED
#define BSTONE_AUDIO_SAMPLE_CONVERTER_INCLUDED

#include <cstdint>

namespace bstone
{

//
// Audio sample converter.
//
struct AudioSampleConverter
{
	static std::int16_t u8_to_s16(std::uint8_t u8_sample) noexcept;
	static float u8_to_f32(std::uint8_t u8_sample) noexcept;
	static double u8_to_f64(std::uint8_t u8_sample) noexcept;
	static float s16_to_f32(std::int16_t s16_sample) noexcept;
	static std::int16_t f32_to_s16(float f32_sample) noexcept;
	static std::int16_t f64_to_s16(double f64_sample) noexcept;
}; // AudioSampleConverter

} // bstone

#endif // !BSTONE_AUDIO_SAMPLE_CONVERTER_INCLUDED
