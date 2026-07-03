/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenAL symbols

#ifndef BSTONE_OAL_SYMBOLS_INCLUDED
#define BSTONE_OAL_SYMBOLS_INCLUDED

#include "al.h"
#include "alc.h"
#include "alext.h"
#include "efx.h"

namespace bstone {

// OpenAL v1.1 ALC symbols

extern LPALCCREATECONTEXT alcCreateContext;
extern LPALCMAKECONTEXTCURRENT alcMakeContextCurrent;
extern LPALCPROCESSCONTEXT alcProcessContext;
extern LPALCSUSPENDCONTEXT alcSuspendContext;
extern LPALCDESTROYCONTEXT alcDestroyContext;
extern LPALCGETCURRENTCONTEXT alcGetCurrentContext;
extern LPALCGETCONTEXTSDEVICE alcGetContextsDevice;
extern LPALCOPENDEVICE alcOpenDevice;
extern LPALCCLOSEDEVICE alcCloseDevice;
extern LPALCGETERROR alcGetError;
extern LPALCISEXTENSIONPRESENT alcIsExtensionPresent;
extern LPALCGETPROCADDRESS alcGetProcAddress;
extern LPALCGETENUMVALUE alcGetEnumValue;
extern LPALCGETSTRING alcGetString;
extern LPALCGETINTEGERV alcGetIntegerv;
extern LPALCCAPTUREOPENDEVICE alcCaptureOpenDevice;
extern LPALCCAPTURECLOSEDEVICE alcCaptureCloseDevice;
extern LPALCCAPTURESTART alcCaptureStart;
extern LPALCCAPTURESTOP alcCaptureStop;
extern LPALCCAPTURESAMPLES alcCaptureSamples;

// OpenAL v1.1 AL symbols

extern LPALDOPPLERFACTOR alDopplerFactor;
extern LPALDOPPLERVELOCITY alDopplerVelocity;
extern LPALSPEEDOFSOUND alSpeedOfSound;
extern LPALDISTANCEMODEL alDistanceModel;
extern LPALENABLE alEnable;
extern LPALDISABLE alDisable;
extern LPALISENABLED alIsEnabled;
extern LPALGETSTRING alGetString;
extern LPALGETBOOLEANV alGetBooleanv;
extern LPALGETINTEGERV alGetIntegerv;
extern LPALGETFLOATV alGetFloatv;
extern LPALGETDOUBLEV alGetDoublev;
extern LPALGETBOOLEAN alGetBoolean;
extern LPALGETINTEGER alGetInteger;
extern LPALGETFLOAT alGetFloat;
extern LPALGETDOUBLE alGetDouble;
extern LPALGETERROR alGetError;
extern LPALISEXTENSIONPRESENT alIsExtensionPresent;
extern LPALGETPROCADDRESS alGetProcAddress;
extern LPALGETENUMVALUE alGetEnumValue;
extern LPALLISTENERF alListenerf;
extern LPALLISTENER3F alListener3f;
extern LPALLISTENERFV alListenerfv;
extern LPALLISTENERI alListeneri;
extern LPALLISTENER3I alListener3i;
extern LPALLISTENERIV alListeneriv;
extern LPALGETLISTENERF alGetListenerf;
extern LPALGETLISTENER3F alGetListener3f;
extern LPALGETLISTENERFV alGetListenerfv;
extern LPALGETLISTENERI alGetListeneri;
extern LPALGETLISTENER3I alGetListener3i;
extern LPALGETLISTENERIV alGetListeneriv;
extern LPALGENSOURCES alGenSources;
extern LPALDELETESOURCES alDeleteSources;
extern LPALISSOURCE alIsSource;
extern LPALSOURCEF alSourcef;
extern LPALSOURCE3F alSource3f;
extern LPALSOURCEFV alSourcefv;
extern LPALSOURCEI alSourcei;
extern LPALSOURCE3I alSource3i;
extern LPALSOURCEIV alSourceiv;
extern LPALGETSOURCEF alGetSourcef;
extern LPALGETSOURCE3F alGetSource3f;
extern LPALGETSOURCEFV alGetSourcefv;
extern LPALGETSOURCEI alGetSourcei;
extern LPALGETSOURCE3I alGetSource3i;
extern LPALGETSOURCEIV alGetSourceiv;
extern LPALSOURCEPLAYV alSourcePlayv;
extern LPALSOURCESTOPV alSourceStopv;
extern LPALSOURCEREWINDV alSourceRewindv;
extern LPALSOURCEPAUSEV alSourcePausev;
extern LPALSOURCEPLAY alSourcePlay;
extern LPALSOURCESTOP alSourceStop;
extern LPALSOURCEREWIND alSourceRewind;
extern LPALSOURCEPAUSE alSourcePause;
extern LPALSOURCEQUEUEBUFFERS alSourceQueueBuffers;
extern LPALSOURCEUNQUEUEBUFFERS alSourceUnqueueBuffers;
extern LPALGENBUFFERS alGenBuffers;
extern LPALDELETEBUFFERS alDeleteBuffers;
extern LPALISBUFFER alIsBuffer;
extern LPALBUFFERDATA alBufferData;
extern LPALBUFFERF alBufferf;
extern LPALBUFFER3F alBuffer3f;
extern LPALBUFFERFV alBufferfv;
extern LPALBUFFERI alBufferi;
extern LPALBUFFER3I alBuffer3i;
extern LPALBUFFERIV alBufferiv;
extern LPALGETBUFFERF alGetBufferf;
extern LPALGETBUFFER3F alGetBuffer3f;
extern LPALGETBUFFERFV alGetBufferfv;
extern LPALGETBUFFERI alGetBufferi;
extern LPALGETBUFFER3I alGetBuffer3i;
extern LPALGETBUFFERIV alGetBufferiv;

// OpenAL v1.1 EFX v1.0 symbols

extern LPALGENEFFECTS alGenEffects;
extern LPALDELETEEFFECTS alDeleteEffects;
extern LPALISEFFECT alIsEffect;
extern LPALEFFECTI alEffecti;
extern LPALEFFECTIV alEffectiv;
extern LPALEFFECTF alEffectf;
extern LPALEFFECTFV alEffectfv;
extern LPALGETEFFECTI alGetEffecti;
extern LPALGETEFFECTIV alGetEffectiv;
extern LPALGETEFFECTF alGetEffectf;
extern LPALGETEFFECTFV alGetEffectfv;
extern LPALGENFILTERS alGenFilters;
extern LPALDELETEFILTERS alDeleteFilters;
extern LPALISFILTER alIsFilter;
extern LPALFILTERI alFilteri;
extern LPALFILTERIV alFilteriv;
extern LPALFILTERF alFilterf;
extern LPALFILTERFV alFilterfv;
extern LPALGETFILTERI alGetFilteri;
extern LPALGETFILTERIV alGetFilteriv;
extern LPALGETFILTERF alGetFilterf;
extern LPALGETFILTERFV alGetFilterfv;
extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
extern LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
extern LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
extern LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
extern LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
extern LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
extern LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
extern LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;

} // namespace bstone

#endif // BSTONE_OAL_SYMBOLS_INCLUDED
