#ifndef BSTONE_VOICE_INCLUDED
#define BSTONE_VOICE_INCLUDED

#include "bstone_audio_mixer_voice_handle.h"

namespace bstone
{

struct Voice
{
	bstone::AudioMixerVoiceHandle handle;
	double gain;
};

} // bstone

#endif // !BSTONE_VOICE_INCLUDED
