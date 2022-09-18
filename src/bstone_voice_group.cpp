/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cassert>
#include <algorithm>
#include <unordered_set>
#include "bstone_exception.h"
#include "bstone_audio_mixer_validator.h"
#include "bstone_voice_group.h"

namespace bstone {

namespace {

class VoiceGroupException : public Exception
{
public:
	explicit VoiceGroupException(const char* message)
		:
		Exception{"VOICE_GROUP", message}
	{}
};

} // namespace

// --------------------------------------------------------------------------

bool VoiceGroup::is_any_playing()
try
{
	return do_is_any_playing();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::set_gain(double gain)
try
{
	do_set_gain(gain);
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::pause()
try
{
	do_pause();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::resume()
try
{
	do_resume();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::stop()
try
{
	do_stop();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::add_voice(Voice& voice)
try
{
	do_add_voice(voice);
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::stop_and_remove_voice(Voice& voice)
try
{
	do_stop_and_remove_voice(voice);
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::stop_voice(Voice& voice)
try
{
	do_stop_voice(voice);
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::set_voice_gain(const Voice& voice)
try
{
	do_set_voice_gain(voice);
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::set_voice_output_gains(const Voice& voice)
try
{
	do_set_voice_output_gains(voice);
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]] void VoiceGroup::fail_nested(const char* message)
{
	std::throw_with_nested(VoiceGroupException{message});
}

// ==========================================================================

namespace {

class VoiceGroupImpl final : public VoiceGroup
{
public:
	VoiceGroupImpl(AudioMixer& audio_mixer);

private:
	using VoiceSet = std::unordered_set<Voice*>;

	AudioMixer* audio_mixer_{};
	VoiceSet voice_set_{};
	double gain_{};

	bool do_is_any_playing() override;
	void do_set_gain(double gain) override;
	void do_pause() override;
	void do_resume() override;
	void do_stop() override;

	void do_add_voice(Voice& voice) override;
	void do_stop_and_remove_voice(Voice& voice) override;
	void do_stop_voice(Voice& voice) override;
	void do_set_voice_gain(const Voice&) override;
	void do_set_voice_output_gains(const Voice& voice) override;
};

// --------------------------------------------------------------------------

VoiceGroupImpl::VoiceGroupImpl(AudioMixer& audio_mixer)
	:
	audio_mixer_{&audio_mixer},
	gain_{audio_mixer_max_gain}
{}

bool VoiceGroupImpl::do_is_any_playing()
{
	for (const auto& voice : voice_set_)
	{
		const auto is_playing = audio_mixer_->is_voice_playing(voice->handle);

		if (is_playing)
		{
			return true;
		}
	}

	return false;
}

void VoiceGroupImpl::do_set_gain(double gain)
{
	AudioMixerValidator::validate_gain(gain);
	gain_ = gain;

	for (const auto& voice : voice_set_)
	{
		voice->use_output_gains ? do_set_voice_output_gains(*voice) : do_set_voice_gain(*voice);
	}
}

void VoiceGroupImpl::do_pause()
{
	for (const auto& voice : voice_set_)
	{
		audio_mixer_->pause_voice(voice->handle);
	}
}

void VoiceGroupImpl::do_resume()
{
	for (const auto& voice : voice_set_)
	{
		audio_mixer_->resume_voice(voice->handle);
	}
}

void VoiceGroupImpl::do_stop()
{
	for (const auto& voice : voice_set_)
	{
		audio_mixer_->stop_voice(voice->handle);
	}

	voice_set_.clear();
}

void VoiceGroupImpl::do_add_voice(Voice& voice)
{
	voice_set_.insert(&voice);
	set_voice_gain(voice);
}

void VoiceGroupImpl::do_stop_and_remove_voice(Voice& voice)
{
	voice_set_.erase(&voice);
}

void VoiceGroupImpl::do_stop_voice(Voice& voice)
{
	const auto voice_handle = voice.handle;
	voice.handle.reset();
	audio_mixer_->stop_voice(voice_handle);
	do_stop_and_remove_voice(voice);
}

void VoiceGroupImpl::do_set_voice_gain(const Voice& voice)
{
	AudioMixerValidator::validate_gain(voice.gain);
	const auto effective_gain = voice.gain * gain_;
	audio_mixer_->set_voice_gain(voice.handle, effective_gain);
}

void VoiceGroupImpl::do_set_voice_output_gains(const Voice& voice)
{
	AudioMixerOutputGains effective_output_gains;

	std::transform(
		voice.output_gains.cbegin(),
		voice.output_gains.cend(),
		effective_output_gains.begin(),
		[this](const double& src_gain)
		{
			return src_gain * gain_;
		}
	);

	audio_mixer_->set_voice_output_gains(voice.handle, effective_output_gains);
}

} // namespace

// ==========================================================================

VoiceGroupUPtr make_voice_group(AudioMixer& audio_mixer)
{
	return std::make_unique<VoiceGroupImpl>(audio_mixer);
}

} // bstone
