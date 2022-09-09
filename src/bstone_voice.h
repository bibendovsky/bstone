#ifndef BSTONE_VOICE_INCLUDED
#define BSTONE_VOICE_INCLUDED

#include "bstone_audio_mixer_output_gains.h"
#include "bstone_audio_mixer_voice_handle.h"

namespace bstone
{

struct Voice
{
	bool use_output_gains;
	bstone::AudioMixerVoiceHandle handle;
	double gain;
	AudioMixerOutputGains output_gains;
};

} // bstone

#endif // !BSTONE_VOICE_INCLUDED
