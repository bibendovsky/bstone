/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <algorithm>
#include "bstone_audio_mixer_validator.h"
#include "bstone_exception.h"

namespace bstone
{

void AudioMixerValidator::validate_gain(double gain)
try {
	if (gain < audio_mixer_min_gain || gain > audio_mixer_max_gain)
	{
		BSTONE_THROW_STATIC_SOURCE("Gain out of range.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void AudioMixerValidator::validate_output_gains(const AudioMixerOutputGains& output_gains)
try {
	const auto is_invalid = std::any_of(
		output_gains.cbegin(),
		output_gains.cend(),
		[](const double& gain)
		{
			return gain < audio_mixer_min_gain || gain > audio_mixer_max_gain;
		}
	);

	if (is_invalid)
	{
		BSTONE_THROW_STATIC_SOURCE("Output gain out of range.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // bstone
