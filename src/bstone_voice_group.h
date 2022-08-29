#ifndef BSTONE_VOICE_GROUP_MGR_INCLUDED
#define BSTONE_VOICE_GROUP_MGR_INCLUDED

#include "bstone_audio_mixer.h"
#include <memory>

namespace bstone
{

class VoiceGroup
{
public:
	VoiceGroup() noexcept = default;
	virtual ~VoiceGroup() = default;

	VoiceGroup& add_group();
	void remove_group(VoiceGroup& group);

	bool is_any_playing();
	void set_gain(double gain);
	void pause();
	void resume();
	void stop();

	void add_voice(AudioMixerVoiceHandle handle);
	void remove_voice(AudioMixerVoiceHandle handle);

	void set_voice_gain(AudioMixerVoiceHandle handle, double gain);

protected:
	[[noreturn]] static void fail_nested(const char* message);

private:
	virtual VoiceGroup& do_add_group() = 0;
	virtual void do_remove_group(VoiceGroup& group) = 0;

	virtual bool do_is_any_playing() = 0;
	virtual void do_set_gain(double gain) = 0;
	virtual void do_pause() = 0;
	virtual void do_resume() = 0;
	virtual void do_stop() = 0;

	virtual void do_add_voice(AudioMixerVoiceHandle handle) = 0;
	virtual void do_remove_voice(AudioMixerVoiceHandle handle) = 0;

	virtual void do_set_voice_gain(AudioMixerVoiceHandle handle, double gain) = 0;
}; // VoiceGroup

// ==========================================================================

using VoiceGroupUPtr = std::unique_ptr<VoiceGroup>;

VoiceGroupUPtr make_voice_group(AudioMixer& audio_mixer);

} // bstone

#endif // !BSTONE_VOICE_GROUP_MGR_INCLUDED
