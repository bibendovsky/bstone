/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#ifndef BSTONE_OAL_SYMBOLS_INCLUDED
#define BSTONE_OAL_SYMBOLS_INCLUDED


#include "al.h"
#include "alc.h"


namespace bstone
{


struct OalSymbols
{
	//
	// AL 1.1
	//

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


	//
	// ALC 1.1
	//

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
}; // OalSymbols


} // bstone


#endif // !BSTONE_OAL_SYMBOLS_INCLUDED
