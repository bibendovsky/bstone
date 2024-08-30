/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// Audio sample converter.
//

#include "bstone_audio_sample_converter.h"
#include <algorithm>

namespace bstone
{

std::int16_t AudioSampleConverter::u8_to_s16(std::uint8_t u8_sample) noexcept
{
	const auto sample_s8 = static_cast<int>(u8_sample) - 128;

	if (sample_s8 < 0)
	{
		// = (value * 32'768) / 128;
		return static_cast<std::int16_t>(sample_s8 * 256);
	}
	else if (sample_s8 > 0)
	{
		return static_cast<std::int16_t>((sample_s8 * 32'767) / 127);
	}
	else
	{
		return 0;
	}
}

float AudioSampleConverter::u8_to_f32(std::uint8_t u8_sample) noexcept
{
	const auto sample_s8 = static_cast<int>(u8_sample) - 128;

	if (sample_s8 < 0)
	{
		return static_cast<float>(sample_s8) / 128.0F;
	}
	else if (sample_s8 > 0)
	{
		return static_cast<float>(sample_s8) / 127.0F;
	}
	else
	{
		return 0.0F;
	}
}

double AudioSampleConverter::u8_to_f64(std::uint8_t u8_sample) noexcept
{
	const auto sample_s8 = static_cast<int>(u8_sample) - 128;

	if (sample_s8 < 0)
	{
		return sample_s8 / 128.0;
	}
	else if (sample_s8 > 0)
	{
		return sample_s8 / 127.0;
	}
	else
	{
		return 0.0;
	}
}

float AudioSampleConverter::s16_to_f32(std::int16_t s16_sample) noexcept
{
	if (s16_sample < 0)
	{
		return s16_sample / 32'768.0F;
	}
	else if (s16_sample > 0)
	{
		return s16_sample / 32'767.0F;
	}
	else
	{
		return 0.0F;
	}
}

std::int16_t AudioSampleConverter::f32_to_s16(float f32_sample) noexcept
{
	if (f32_sample < 0.0F)
	{
		return static_cast<std::int16_t>(std::max(static_cast<int>(f32_sample * 32'768.0F), -32'768));
	}
	else if (f32_sample > 0.0F)
	{
		return static_cast<std::int16_t>(std::min(static_cast<int>(f32_sample * 32'767.0F), 32'767));
	}
	else
	{
		return 0;
	}
}

std::int16_t AudioSampleConverter::f64_to_s16(double f64_sample) noexcept
{
	if (f64_sample < 0.0)
	{
		return static_cast<std::int16_t>(std::max(static_cast<int>(f64_sample * 32'768.0), -32'768));
	}
	else if (f64_sample > 0.0)
	{
		return static_cast<std::int16_t>(std::min(static_cast<int>(f64_sample * 32'767.0), 32'767));
	}
	else
	{
		return 0;
	}
}

} // bstone
