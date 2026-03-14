/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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

class VoiceGroupImpl final : public VoiceGroup
{
public:
	VoiceGroupImpl(AudioMixer& audio_mixer);
	~VoiceGroupImpl() override = default;

	bool is_any_playing() override;
	void set_gain(double gain) override;
	void pause() override;
	void resume() override;
	void stop() override;

	void add_voice(Voice& voice) override;
	void stop_and_remove_voice(Voice& voice) override;
	void stop_voice(Voice& voice) override;
	void set_voice_gain(const Voice&) override;
	void set_voice_output_gains(const Voice& voice) override;

private:
	using VoiceSet = std::unordered_set<Voice*>;

	AudioMixer* audio_mixer_{};
	VoiceSet voice_set_{};
	double gain_{};
};

// --------------------------------------------------------------------------

VoiceGroupImpl::VoiceGroupImpl(AudioMixer& audio_mixer)
	:
	audio_mixer_{&audio_mixer},
	gain_{audio_mixer_max_gain}
{}

bool VoiceGroupImpl::is_any_playing()
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

void VoiceGroupImpl::set_gain(double gain)
{
	AudioMixerValidator::validate_gain(gain);
	gain_ = gain;

	for (const auto& voice : voice_set_)
	{
		voice->use_output_gains ? set_voice_output_gains(*voice) : set_voice_gain(*voice);
	}
}

void VoiceGroupImpl::pause()
{
	for (const auto& voice : voice_set_)
	{
		audio_mixer_->pause_voice(voice->handle);
	}
}

void VoiceGroupImpl::resume()
{
	for (const auto& voice : voice_set_)
	{
		audio_mixer_->resume_voice(voice->handle);
	}
}

void VoiceGroupImpl::stop()
{
	for (const auto& voice : voice_set_)
	{
		audio_mixer_->stop_voice(voice->handle);
	}

	voice_set_.clear();
}

void VoiceGroupImpl::add_voice(Voice& voice)
{
	voice_set_.insert(&voice);
	set_voice_gain(voice);
}

void VoiceGroupImpl::stop_and_remove_voice(Voice& voice)
{
	voice_set_.erase(&voice);
}

void VoiceGroupImpl::stop_voice(Voice& voice)
{
	const auto voice_handle = voice.handle;
	voice.handle.reset();
	audio_mixer_->stop_voice(voice_handle);
	stop_and_remove_voice(voice);
}

void VoiceGroupImpl::set_voice_gain(const Voice& voice)
{
	AudioMixerValidator::validate_gain(voice.gain);
	const auto effective_gain = voice.gain * gain_;
	audio_mixer_->set_voice_gain(voice.handle, effective_gain);
}

void VoiceGroupImpl::set_voice_output_gains(const Voice& voice)
{
	auto effective_output_gains = AudioMixerOutputGains{};

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
