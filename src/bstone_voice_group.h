#ifndef BSTONE_VOICE_GROUP_INCLUDED
#define BSTONE_VOICE_GROUP_INCLUDED

#include "bstone_audio_mixer.h"
#include "bstone_voice.h"
#include <memory>

namespace bstone {

class VoiceGroup
{
public:
	VoiceGroup() noexcept = default;
	virtual ~VoiceGroup() = default;

	bool is_any_playing();
	void set_gain(double gain);
	void pause();
	void resume();
	void stop();

	void add_voice(Voice& voice);
	void stop_and_remove_voice(Voice& voice);
	void stop_voice(Voice& voice);
	void set_voice_gain(const Voice& voice);
	void set_voice_output_gains(const Voice& voice);

protected:
	[[noreturn]] static void fail_nested(const char* message);

private:
	virtual bool do_is_any_playing() = 0;
	virtual void do_set_gain(double gain) = 0;
	virtual void do_pause() = 0;
	virtual void do_resume() = 0;
	virtual void do_stop() = 0;

	virtual void do_add_voice(Voice& voice) = 0;
	virtual void do_stop_and_remove_voice(Voice& voice) = 0;
	virtual void do_stop_voice(Voice& voice) = 0;
	virtual void do_set_voice_gain(const Voice& voice) = 0;
	virtual void do_set_voice_output_gains(const Voice& voice) = 0;
};

// ==========================================================================

using VoiceGroupUPtr = std::unique_ptr<VoiceGroup>;

VoiceGroupUPtr make_voice_group(AudioMixer& audio_mixer);

} // bstone

#endif // !BSTONE_VOICE_GROUP_INCLUDED
