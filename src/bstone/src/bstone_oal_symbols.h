/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_OAL_SYMBOLS_INCLUDED
#define BSTONE_OAL_SYMBOLS_INCLUDED

#include "al.h"
#include "alc.h"
#include "efx.h"

namespace bstone
{

struct OalAlSymbols
{
	// OpenAL v1.1 ALC symbols

	LPALCCREATECONTEXT alcCreateContext;
	LPALCMAKECONTEXTCURRENT alcMakeContextCurrent;
	LPALCPROCESSCONTEXT alcProcessContext;
	LPALCSUSPENDCONTEXT alcSuspendContext;
	LPALCDESTROYCONTEXT alcDestroyContext;
	LPALCGETCURRENTCONTEXT alcGetCurrentContext;
	LPALCGETCONTEXTSDEVICE alcGetContextsDevice;
	LPALCOPENDEVICE alcOpenDevice;
	LPALCCLOSEDEVICE alcCloseDevice;
	LPALCGETERROR alcGetError;
	LPALCISEXTENSIONPRESENT alcIsExtensionPresent;
	LPALCGETPROCADDRESS alcGetProcAddress;
	LPALCGETENUMVALUE alcGetEnumValue;
	LPALCGETSTRING alcGetString;
	LPALCGETINTEGERV alcGetIntegerv;
	LPALCCAPTUREOPENDEVICE alcCaptureOpenDevice;
	LPALCCAPTURECLOSEDEVICE alcCaptureCloseDevice;
	LPALCCAPTURESTART alcCaptureStart;
	LPALCCAPTURESTOP alcCaptureStop;
	LPALCCAPTURESAMPLES alcCaptureSamples;

	// OpenAL v1.1 AL symbols

	LPALDOPPLERFACTOR alDopplerFactor;
	LPALDOPPLERVELOCITY alDopplerVelocity;
	LPALSPEEDOFSOUND alSpeedOfSound;
	LPALDISTANCEMODEL alDistanceModel;
	LPALENABLE alEnable;
	LPALDISABLE alDisable;
	LPALISENABLED alIsEnabled;
	LPALGETSTRING alGetString;
	LPALGETBOOLEANV alGetBooleanv;
	LPALGETINTEGERV alGetIntegerv;
	LPALGETFLOATV alGetFloatv;
	LPALGETDOUBLEV alGetDoublev;
	LPALGETBOOLEAN alGetBoolean;
	LPALGETINTEGER alGetInteger;
	LPALGETFLOAT alGetFloat;
	LPALGETDOUBLE alGetDouble;
	LPALGETERROR alGetError;
	LPALISEXTENSIONPRESENT alIsExtensionPresent;
	LPALGETPROCADDRESS alGetProcAddress;
	LPALGETENUMVALUE alGetEnumValue;
	LPALLISTENERF alListenerf;
	LPALLISTENER3F alListener3f;
	LPALLISTENERFV alListenerfv;
	LPALLISTENERI alListeneri;
	LPALLISTENER3I alListener3i;
	LPALLISTENERIV alListeneriv;
	LPALGETLISTENERF alGetListenerf;
	LPALGETLISTENER3F alGetListener3f;
	LPALGETLISTENERFV alGetListenerfv;
	LPALGETLISTENERI alGetListeneri;
	LPALGETLISTENER3I alGetListener3i;
	LPALGETLISTENERIV alGetListeneriv;
	LPALGENSOURCES alGenSources;
	LPALDELETESOURCES alDeleteSources;
	LPALISSOURCE alIsSource;
	LPALSOURCEF alSourcef;
	LPALSOURCE3F alSource3f;
	LPALSOURCEFV alSourcefv;
	LPALSOURCEI alSourcei;
	LPALSOURCE3I alSource3i;
	LPALSOURCEIV alSourceiv;
	LPALGETSOURCEF alGetSourcef;
	LPALGETSOURCE3F alGetSource3f;
	LPALGETSOURCEFV alGetSourcefv;
	LPALGETSOURCEI alGetSourcei;
	LPALGETSOURCE3I alGetSource3i;
	LPALGETSOURCEIV alGetSourceiv;
	LPALSOURCEPLAYV alSourcePlayv;
	LPALSOURCESTOPV alSourceStopv;
	LPALSOURCEREWINDV alSourceRewindv;
	LPALSOURCEPAUSEV alSourcePausev;
	LPALSOURCEPLAY alSourcePlay;
	LPALSOURCESTOP alSourceStop;
	LPALSOURCEREWIND alSourceRewind;
	LPALSOURCEPAUSE alSourcePause;
	LPALSOURCEQUEUEBUFFERS alSourceQueueBuffers;
	LPALSOURCEUNQUEUEBUFFERS alSourceUnqueueBuffers;
	LPALGENBUFFERS alGenBuffers;
	LPALDELETEBUFFERS alDeleteBuffers;
	LPALISBUFFER alIsBuffer;
	LPALBUFFERDATA alBufferData;
	LPALBUFFERF alBufferf;
	LPALBUFFER3F alBuffer3f;
	LPALBUFFERFV alBufferfv;
	LPALBUFFERI alBufferi;
	LPALBUFFER3I alBuffer3i;
	LPALBUFFERIV alBufferiv;
	LPALGETBUFFERF alGetBufferf;
	LPALGETBUFFER3F alGetBuffer3f;
	LPALGETBUFFERFV alGetBufferfv;
	LPALGETBUFFERI alGetBufferi;
	LPALGETBUFFER3I alGetBuffer3i;
	LPALGETBUFFERIV alGetBufferiv;

	// OpenAL v1.1 EFX v1.0 symbols

	LPALGENEFFECTS alGenEffects;
	LPALDELETEEFFECTS alDeleteEffects;
	LPALISEFFECT alIsEffect;
	LPALEFFECTI alEffecti;
	LPALEFFECTIV alEffectiv;
	LPALEFFECTF alEffectf;
	LPALEFFECTFV alEffectfv;
	LPALGETEFFECTI alGetEffecti;
	LPALGETEFFECTIV alGetEffectiv;
	LPALGETEFFECTF alGetEffectf;
	LPALGETEFFECTFV alGetEffectfv;
	LPALGENFILTERS alGenFilters;
	LPALDELETEFILTERS alDeleteFilters;
	LPALISFILTER alIsFilter;
	LPALFILTERI alFilteri;
	LPALFILTERIV alFilteriv;
	LPALFILTERF alFilterf;
	LPALFILTERFV alFilterfv;
	LPALGETFILTERI alGetFilteri;
	LPALGETFILTERIV alGetFilteriv;
	LPALGETFILTERF alGetFilterf;
	LPALGETFILTERFV alGetFilterfv;
	LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
	LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
	LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
	LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
	LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
	LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
	LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
	LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
	LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
	LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
	LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;
}; // OalAlSymbols

} // bstone

#endif // !BSTONE_OAL_SYMBOLS_INCLUDED
