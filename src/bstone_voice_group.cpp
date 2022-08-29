#include "bstone_voice_group.h"

#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "bstone_exception.h"
#include "bstone_audio_mixer_validator.h"

namespace bstone
{

namespace
{

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

VoiceGroup& VoiceGroup::add_group()
try
{
	return do_add_group();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::remove_group(VoiceGroup& group)
try
{
	do_remove_group(group);
}
catch (...)
{
	fail_nested(__func__);
}

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

void VoiceGroup::add_voice(AudioMixerVoiceHandle handle)
try
{
	do_add_voice(handle);
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::remove_voice(AudioMixerVoiceHandle handle)
try
{
	do_remove_voice(handle);
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroup::set_voice_gain(AudioMixerVoiceHandle handle, double gain)
try
{
	do_set_voice_gain(handle, gain);
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

namespace
{

class VoiceGroupImpl final : public VoiceGroup
{
public:
	VoiceGroupImpl(AudioMixer& audio_mixer) noexcept;
	VoiceGroupImpl(AudioMixer& audio_mixer, VoiceGroupImpl& parent_group) noexcept;

	double get_effective_gain() const noexcept;
	void on_parent_set_gain();
	void on_parent_pause();
	void on_parent_resume();
	void on_parent_stop();

private:
	using VoiceGroupImplUPtr = std::unique_ptr<VoiceGroupImpl>;
	using Subgroups = std::unordered_set<VoiceGroupImplUPtr>;
	using VoiceGainMap = std::unordered_map<AudioMixerVoiceHandle, double, AudioMixerVoiceHandleStdHasher>;

	AudioMixer* audio_mixer_{};
	VoiceGroupImpl* parent_group_{};
	Subgroups subgroups_{};
	VoiceGainMap voice_gain_map_{};
	double gain_{};
	double effective_gain_{};

	void update_effective_gain();
	void stop_and_remove_voices();
	void pause_voices();
	void resume_voices();

	VoiceGroup& do_add_group() override;
	void do_remove_group(VoiceGroup& group) override;

	bool do_is_any_playing() override;
	void do_set_gain(double gain) override;
	void do_pause() override;
	void do_resume() override;
	void do_stop() override;

	void do_add_voice(AudioMixerVoiceHandle handle) override;
	void do_remove_voice(AudioMixerVoiceHandle handle) override;

	void do_set_voice_gain(AudioMixerVoiceHandle handle, double gain) override;
}; // VoiceGroupImpl

// --------------------------------------------------------------------------

VoiceGroupImpl::VoiceGroupImpl(AudioMixer& audio_mixer) noexcept
	:
	audio_mixer_{&audio_mixer}
{
}

VoiceGroupImpl::VoiceGroupImpl(AudioMixer& audio_mixer, VoiceGroupImpl& parent_group) noexcept
	:
	audio_mixer_{&audio_mixer},
	parent_group_{&parent_group}
{
}

double VoiceGroupImpl::get_effective_gain() const noexcept
{
	return effective_gain_;
}

void VoiceGroupImpl::on_parent_set_gain()
try
{
	update_effective_gain();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::on_parent_pause()
try
{
	pause_voices();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::on_parent_resume()
try
{
	resume_voices();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::on_parent_stop()
try
{
	stop_and_remove_voices();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::update_effective_gain()
try
{
	effective_gain_ = gain_;

	if (parent_group_)
	{
		effective_gain_ *= parent_group_->get_effective_gain();
	}

	for (const auto voice_gain_pair : voice_gain_map_)
	{
		const auto voice_gain = voice_gain_pair.second * get_effective_gain();
		audio_mixer_->set_voice_gain(voice_gain_pair.first, voice_gain);
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::stop_and_remove_voices()
try
{
	for (auto voice_gain_pair : voice_gain_map_)
	{
		audio_mixer_->stop_voice(voice_gain_pair.first);
	}

	voice_gain_map_.clear();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::pause_voices()
try
{
	for (const auto voice_gain_pair : voice_gain_map_)
	{
		audio_mixer_->pause_voice(voice_gain_pair.first);
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::resume_voices()
try
{
	for (auto voice_gain_pair : voice_gain_map_)
	{
		audio_mixer_->resume_voice(voice_gain_pair.first);
	}
}
catch (...)
{
	fail_nested(__func__);
}

VoiceGroup& VoiceGroupImpl::do_add_group()
try
{
	return **subgroups_.emplace(std::make_unique<VoiceGroupImpl>(*audio_mixer_, *this)).first;
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::do_remove_group(VoiceGroup& group)
try
{
	auto dummy_uptr = VoiceGroupImplUPtr{&static_cast<VoiceGroupImpl&>(group)};
	subgroups_.erase(dummy_uptr);
	dummy_uptr.release();
}
catch (...)
{
	fail_nested(__func__);
}

bool VoiceGroupImpl::do_is_any_playing()
try
{
	for (const auto voice_gain_pair : voice_gain_map_)
	{
		const auto is_playing = audio_mixer_->is_voice_playing(voice_gain_pair.first);

		if (is_playing)
		{
			return true;
		}
	}

	return false;
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::do_set_gain(double gain)
try
{
	AudioMixerValidator::validate_gain(gain);

	if (gain_ == gain)
	{
		return;
	}

	gain_ = gain;
	update_effective_gain();

	for (auto& subgroup : subgroups_)
	{
		subgroup->on_parent_set_gain();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::do_pause()
try
{
	pause_voices();

	for (auto& sub_group : subgroups_)
	{
		sub_group->on_parent_pause();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::do_resume()
try
{
	resume_voices();

	for (auto& sub_group : subgroups_)
	{
		sub_group->on_parent_resume();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::do_stop()
try
{
	stop_and_remove_voices();

	for (auto& sub_group : subgroups_)
	{
		sub_group->on_parent_stop();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::do_add_voice(AudioMixerVoiceHandle handle)
try
{
	if (!handle.is_valid())
	{
		return;
	}

	voice_gain_map_.emplace(handle, 1.0);
	audio_mixer_->set_voice_gain(handle, get_effective_gain());
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::do_remove_voice(AudioMixerVoiceHandle handle)
try
{
	voice_gain_map_.erase(handle);
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::do_set_voice_gain(AudioMixerVoiceHandle handle, double gain)
try
{
	AudioMixerValidator::validate_gain(gain);
	const auto voice_gain_it = voice_gain_map_.find(handle);

	if (voice_gain_it == voice_gain_map_.end())
	{
		return;
	}

	voice_gain_it->second = gain;
	const auto voice_gain = gain * get_effective_gain();
	audio_mixer_->set_voice_gain(handle, voice_gain);
}
catch (...)
{
	fail_nested(__func__);
}

} // namespace

// ==========================================================================

VoiceGroupUPtr make_voice_group(AudioMixer& audio_mixer)
{
	return std::make_unique<VoiceGroupImpl>(audio_mixer);
}

} // bstone
