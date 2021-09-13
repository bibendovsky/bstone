#include "bstone_voice_group.h"

#include <cassert>
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
	{
	}
}; // VoiceGroupException

// ==========================================================================

class VoiceGroupImpl final : public VoiceGroup
{
public:
	VoiceGroupImpl(AudioMixer& audio_mixer) noexcept;
	VoiceGroupImpl(AudioMixer& audio_mixer, VoiceGroupImpl& parent_group) noexcept;

	VoiceGroup& add_group() override;
	void remove_group(VoiceGroup& group) override;

	bool is_any_playing() override;
	void set_gain(double gain) override;
	void pause() override;
	void resume() override;
	void stop() override;

	void add_voice(AudioMixerVoiceHandle handle) override;
	void remove_voice(AudioMixerVoiceHandle handle) override;

	double get_effective_gain() const noexcept;
	void on_parent_set_gain();
	void on_parent_pause();
	void on_parent_resume();
	void on_parent_stop();

private:
	using VoiceGroupImplUPtr = std::unique_ptr<VoiceGroupImpl>;
	using SubGroups = std::unordered_set<VoiceGroupImplUPtr>;
	using VoiceHandles = std::unordered_set<AudioMixerVoiceHandle, AudioMixerVoiceHandleStdHasher>;

	AudioMixer* audio_mixer_{};
	VoiceGroupImpl* parent_group_{};
	SubGroups sub_groups_{};
	VoiceHandles voice_handles_{};
	double gain_{};
	double effective_gain_{};

	[[noreturn]] static void fail_nested(const char* message);

	void update_effective_gain();
	void stop_and_remove_voices();
	void pause_voices();
	void resume_voices();
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

VoiceGroup& VoiceGroupImpl::add_group()
try
{
	return **sub_groups_.emplace(std::make_unique<VoiceGroupImpl>(*audio_mixer_, *this)).first;
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::remove_group(VoiceGroup& group)
try
{
	auto dummy_uptr = VoiceGroupImplUPtr{&static_cast<VoiceGroupImpl&>(group)};
	sub_groups_.erase(dummy_uptr);
	dummy_uptr.release();
}
catch (...)
{
	fail_nested(__func__);
}

bool VoiceGroupImpl::is_any_playing()
try
{
	for (auto voice_handle : voice_handles_)
	{
		const auto is_playing = audio_mixer_->is_voice_playing(voice_handle);

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

void VoiceGroupImpl::set_gain(double gain)
try
{
	AudioMixerValidator::validate_gain(gain);

	if (gain_ == gain)
	{
		return;
	}

	gain_ = gain;
	update_effective_gain();

	for (auto& sub_group : sub_groups_)
	{
		sub_group->on_parent_set_gain();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::pause()
try
{
	pause_voices();

	for (auto& sub_group : sub_groups_)
	{
		sub_group->on_parent_pause();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::resume()
try
{
	resume_voices();

	for (auto& sub_group : sub_groups_)
	{
		sub_group->on_parent_resume();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::stop()
try
{
	stop_and_remove_voices();

	for (auto& sub_group : sub_groups_)
	{
		sub_group->on_parent_stop();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::add_voice(AudioMixerVoiceHandle handle)
try
{
	if (!handle.is_valid())
	{
		return;
	}

	voice_handles_.emplace(handle);
	audio_mixer_->set_voice_gain(handle, get_effective_gain());
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::remove_voice(AudioMixerVoiceHandle handle)
try
{
	voice_handles_.erase(handle);
}
catch (...)
{
	fail_nested(__func__);
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

[[noreturn]] void VoiceGroupImpl::fail_nested(const char* message)
{
	std::throw_with_nested(VoiceGroupException{message});
}

void VoiceGroupImpl::update_effective_gain()
try
{
	effective_gain_ = gain_;

	if (parent_group_)
	{
		effective_gain_ *= parent_group_->get_effective_gain();
	}

	for (auto voice_handle : voice_handles_)
	{
		audio_mixer_->set_voice_gain(voice_handle, get_effective_gain());
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::stop_and_remove_voices()
try
{
	for (auto voice_handle : voice_handles_)
	{
		audio_mixer_->stop_voice(voice_handle);
	}

	voice_handles_.clear();
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::pause_voices()
try
{
	for (auto voice_handle : voice_handles_)
	{
		audio_mixer_->pause_voice(voice_handle);
	}
}
catch (...)
{
	fail_nested(__func__);
}

void VoiceGroupImpl::resume_voices()
try
{
	for (auto voice_handle : voice_handles_)
	{
		audio_mixer_->resume_voice(voice_handle);
	}
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
