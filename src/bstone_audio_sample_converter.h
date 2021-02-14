/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
	static std::int16_t u8_to_s16(
		const std::uint8_t u8_sample) noexcept;

	static float u8_to_f32(
		const std::uint8_t u8_sample) noexcept;

	static double u8_to_f64(
		const std::uint8_t u8_sample) noexcept;

	static float s16_to_f32(
		const std::int16_t s16_sample) noexcept;

	static std::int16_t f32_to_s16(
		const float f32_sample) noexcept;

	static std::int16_t f64_to_s16(
		const double f64_sample) noexcept;
}; // AudioSampleConverter


} // bstone


#endif // !BSTONE_AUDIO_SAMPLE_CONVERTER_INCLUDED
