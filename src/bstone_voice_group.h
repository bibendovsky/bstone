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

	virtual VoiceGroup& add_group() = 0;
	virtual void remove_group(VoiceGroup& group) = 0;

	virtual bool is_any_playing() = 0;
	virtual void set_gain(double gain) = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void stop() = 0;

	virtual void add_voice(AudioMixerVoiceHandle handle) = 0;
	virtual void remove_voice(AudioMixerVoiceHandle handle) = 0;
}; // VoiceGroup

// ==========================================================================

using VoiceGroupUPtr = std::unique_ptr<VoiceGroup>;

VoiceGroupUPtr make_voice_group(AudioMixer& audio_mixer);

} // bstone

#endif // !BSTONE_VOICE_GROUP_MGR_INCLUDED
