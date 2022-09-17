/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <algorithm>
#include "bstone_audio_mixer_validator.h"
#include "bstone_exception.h"

namespace bstone
{

namespace
{

class AudioMixerValidatorException : public Exception
{
public:
	explicit AudioMixerValidatorException(const char* message) noexcept
		:
		Exception{"AUDIO_MIXER_VALIDATOR", message}
	{
	}
}; // AudioMixerValidatorException

} // namespace

// ==========================================================================

void AudioMixerValidator::validate_gain(double gain)
try
{
	if (gain < audio_mixer_min_gain || gain > audio_mixer_max_gain)
	{
		fail("Gain out of range.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void AudioMixerValidator::validate_output_gains(const AudioMixerOutputGains& output_gains)
try
{
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
		fail("Output gain out of range.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]] void AudioMixerValidator::fail(const char* message)
{
	throw AudioMixerValidatorException{message};
}

[[noreturn]] void AudioMixerValidator::fail_nested(const char* message)
{
	std::throw_with_nested(AudioMixerValidatorException{message});
}

} // bstone
