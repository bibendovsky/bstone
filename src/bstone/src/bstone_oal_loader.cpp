/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_oal_loader.h"
#include "bstone_assert.h"
#include "bstone_shared_library.h"
#include "bstone_exception.h"
#include <exception>
#include <string>
#include <utility>

namespace bstone
{

class OalLoaderImpl final : public OalLoader
{
public:
	OalLoaderImpl(const char* shared_library_path);
	~OalLoaderImpl() override = default;

	void load_alc_symbols() override;
	void load_al_symbols() override;
	void load_efx_symbols() override;

private:
	SharedLibrary shared_library_{};

	LPALCGETPROCADDRESS alcGetProcAddress_{};
	LPALGETPROCADDRESS alGetProcAddress_{};

	void load_essential_symbols();
	void open_internal(const char* path);

	template<typename T>
	void find_symbol(const char* name, T& symbol)
	{
		BSTONE_ASSERT(name != nullptr && *name != '\0');
		symbol = shared_library_.find_symbol<T>(name);
		if (symbol == nullptr)
		{
			const std::string message = std::string{} + "Symbol \"" + name + "\" not found.";
			BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
		}
	}

	template<
		typename TSymbol,
		typename TFuncResult,
		typename ...TFuncArgs,
		typename ...TArgs
	>
	void find_alx_symbol(const char* name, TSymbol& symbol, const char* acronym, TFuncResult (*func)(TFuncArgs...), TArgs... func_args)
	{
		BSTONE_ASSERT(name != nullptr && *name != '\0');
		BSTONE_ASSERT(acronym != nullptr && *acronym != '\0');
		BSTONE_ASSERT(func != nullptr);
		symbol = reinterpret_cast<TSymbol>(func(std::forward<TFuncArgs>(func_args)...));
		if (symbol == nullptr)
		{
			const std::string message = std::string{} + acronym + " symbol \"" + name + "\" not found.";
			BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
		}
	}

	template<typename TSymbol>
	void find_alc_symbol(const char* name, TSymbol& symbol)
	{
		find_alx_symbol(name, symbol, "ALC", alcGetProcAddress_, static_cast<ALCdevice*>(nullptr), name);
	}

	template<typename TSymbol>
	void find_al_symbol(const char* name, TSymbol& symbol)
	{
		find_alx_symbol(name, symbol, "AL", alGetProcAddress_, name);
	}
};

// ==========================================================================

OalLoaderImpl::OalLoaderImpl(const char* shared_library_path)
{
	open_internal(shared_library_path);
}

void OalLoaderImpl::load_alc_symbols()
try
{
	find_alc_symbol("alcCreateContext", alcCreateContext);
	find_alc_symbol("alcMakeContextCurrent", alcMakeContextCurrent);
	find_alc_symbol("alcProcessContext", alcProcessContext);
	find_alc_symbol("alcSuspendContext", alcSuspendContext);
	find_alc_symbol("alcDestroyContext", alcDestroyContext);
	find_alc_symbol("alcGetCurrentContext", alcGetCurrentContext);
	find_alc_symbol("alcGetContextsDevice", alcGetContextsDevice);
	find_alc_symbol("alcOpenDevice", alcOpenDevice);
	find_alc_symbol("alcCloseDevice", alcCloseDevice);
	find_alc_symbol("alcGetError", alcGetError);
	find_alc_symbol("alcIsExtensionPresent", alcIsExtensionPresent);
	find_alc_symbol("alcGetProcAddress", alcGetProcAddress);
	find_alc_symbol("alcGetEnumValue", alcGetEnumValue);
	find_alc_symbol("alcGetString", alcGetString);
	find_alc_symbol("alcGetIntegerv", alcGetIntegerv);
	find_alc_symbol("alcCaptureOpenDevice", alcCaptureOpenDevice);
	find_alc_symbol("alcCaptureCloseDevice", alcCaptureCloseDevice);
	find_alc_symbol("alcCaptureStart", alcCaptureStart);
	find_alc_symbol("alcCaptureStop", alcCaptureStop);
	find_alc_symbol("alcCaptureSamples", alcCaptureSamples);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalLoaderImpl::load_al_symbols()
try
{
	find_al_symbol("alDopplerFactor", alDopplerFactor);
	find_al_symbol("alDopplerVelocity", alDopplerVelocity);
	find_al_symbol("alSpeedOfSound", alSpeedOfSound);
	find_al_symbol("alDistanceModel", alDistanceModel);
	find_al_symbol("alEnable", alEnable);
	find_al_symbol("alDisable", alDisable);
	find_al_symbol("alIsEnabled", alIsEnabled);
	find_al_symbol("alGetString", alGetString);
	find_al_symbol("alGetBooleanv", alGetBooleanv);
	find_al_symbol("alGetIntegerv", alGetIntegerv);
	find_al_symbol("alGetFloatv", alGetFloatv);
	find_al_symbol("alGetDoublev", alGetDoublev);
	find_al_symbol("alGetBoolean", alGetBoolean);
	find_al_symbol("alGetInteger", alGetInteger);
	find_al_symbol("alGetFloat", alGetFloat);
	find_al_symbol("alGetDouble", alGetDouble);
	find_al_symbol("alGetError", alGetError);
	find_al_symbol("alIsExtensionPresent", alIsExtensionPresent);
	find_al_symbol("alGetProcAddress", alGetProcAddress);
	find_al_symbol("alGetEnumValue", alGetEnumValue);
	find_al_symbol("alListenerf", alListenerf);
	find_al_symbol("alListener3f", alListener3f);
	find_al_symbol("alListenerfv", alListenerfv);
	find_al_symbol("alListeneri", alListeneri);
	find_al_symbol("alListener3i", alListener3i);
	find_al_symbol("alListeneriv", alListeneriv);
	find_al_symbol("alGetListenerf", alGetListenerf);
	find_al_symbol("alGetListener3f", alGetListener3f);
	find_al_symbol("alGetListenerfv", alGetListenerfv);
	find_al_symbol("alGetListeneri", alGetListeneri);
	find_al_symbol("alGetListener3i", alGetListener3i);
	find_al_symbol("alGetListeneriv", alGetListeneriv);
	find_al_symbol("alGenSources", alGenSources);
	find_al_symbol("alDeleteSources", alDeleteSources);
	find_al_symbol("alIsSource", alIsSource);
	find_al_symbol("alSourcef", alSourcef);
	find_al_symbol("alSource3f", alSource3f);
	find_al_symbol("alSourcefv", alSourcefv);
	find_al_symbol("alSourcei", alSourcei);
	find_al_symbol("alSource3i", alSource3i);
	find_al_symbol("alSourceiv", alSourceiv);
	find_al_symbol("alGetSourcef", alGetSourcef);
	find_al_symbol("alGetSource3f", alGetSource3f);
	find_al_symbol("alGetSourcefv", alGetSourcefv);
	find_al_symbol("alGetSourcei", alGetSourcei);
	find_al_symbol("alGetSource3i", alGetSource3i);
	find_al_symbol("alGetSourceiv", alGetSourceiv);
	find_al_symbol("alSourcePlayv", alSourcePlayv);
	find_al_symbol("alSourceStopv", alSourceStopv);
	find_al_symbol("alSourceRewindv", alSourceRewindv);
	find_al_symbol("alSourcePausev", alSourcePausev);
	find_al_symbol("alSourcePlay", alSourcePlay);
	find_al_symbol("alSourceStop", alSourceStop);
	find_al_symbol("alSourceRewind", alSourceRewind);
	find_al_symbol("alSourcePause", alSourcePause);
	find_al_symbol("alSourceQueueBuffers", alSourceQueueBuffers);
	find_al_symbol("alSourceUnqueueBuffers", alSourceUnqueueBuffers);
	find_al_symbol("alGenBuffers", alGenBuffers);
	find_al_symbol("alDeleteBuffers", alDeleteBuffers);
	find_al_symbol("alIsBuffer", alIsBuffer);
	find_al_symbol("alBufferData", alBufferData);
	find_al_symbol("alBufferf", alBufferf);
	find_al_symbol("alBuffer3f", alBuffer3f);
	find_al_symbol("alBufferfv", alBufferfv);
	find_al_symbol("alBufferi", alBufferi);
	find_al_symbol("alBuffer3i", alBuffer3i);
	find_al_symbol("alBufferiv", alBufferiv);
	find_al_symbol("alGetBufferf", alGetBufferf);
	find_al_symbol("alGetBuffer3f", alGetBuffer3f);
	find_al_symbol("alGetBufferfv", alGetBufferfv);
	find_al_symbol("alGetBufferi", alGetBufferi);
	find_al_symbol("alGetBuffer3i", alGetBuffer3i);
	find_al_symbol("alGetBufferiv", alGetBufferiv);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalLoaderImpl::load_efx_symbols()
try
{
	find_al_symbol("alGenEffects", alGenEffects);
	find_al_symbol("alDeleteEffects", alDeleteEffects);
	find_al_symbol("alIsEffect", alIsEffect);
	find_al_symbol("alEffecti", alEffecti);
	find_al_symbol("alEffectiv", alEffectiv);
	find_al_symbol("alEffectf", alEffectf);
	find_al_symbol("alEffectfv", alEffectfv);
	find_al_symbol("alGetEffecti", alGetEffecti);
	find_al_symbol("alGetEffectiv", alGetEffectiv);
	find_al_symbol("alGetEffectf", alGetEffectf);
	find_al_symbol("alGetEffectfv", alGetEffectfv);
	find_al_symbol("alGenFilters", alGenFilters);
	find_al_symbol("alDeleteFilters", alDeleteFilters);
	find_al_symbol("alIsFilter", alIsFilter);
	find_al_symbol("alFilteri", alFilteri);
	find_al_symbol("alFilteriv", alFilteriv);
	find_al_symbol("alFilterf", alFilterf);
	find_al_symbol("alFilterfv", alFilterfv);
	find_al_symbol("alGetFilteri", alGetFilteri);
	find_al_symbol("alGetFilteriv", alGetFilteriv);
	find_al_symbol("alGetFilterf", alGetFilterf);
	find_al_symbol("alGetFilterfv", alGetFilterfv);
	find_al_symbol("alGenAuxiliaryEffectSlots", alGenAuxiliaryEffectSlots);
	find_al_symbol("alDeleteAuxiliaryEffectSlots", alDeleteAuxiliaryEffectSlots);
	find_al_symbol("alIsAuxiliaryEffectSlot", alIsAuxiliaryEffectSlot);
	find_al_symbol("alAuxiliaryEffectSloti", alAuxiliaryEffectSloti);
	find_al_symbol("alAuxiliaryEffectSlotiv", alAuxiliaryEffectSlotiv);
	find_al_symbol("alAuxiliaryEffectSlotf", alAuxiliaryEffectSlotf);
	find_al_symbol("alAuxiliaryEffectSlotfv", alAuxiliaryEffectSlotfv);
	find_al_symbol("alGetAuxiliaryEffectSloti", alGetAuxiliaryEffectSloti);
	find_al_symbol("alGetAuxiliaryEffectSlotiv", alGetAuxiliaryEffectSlotiv);
	find_al_symbol("alGetAuxiliaryEffectSlotf", alGetAuxiliaryEffectSlotf);
	find_al_symbol("alGetAuxiliaryEffectSlotfv", alGetAuxiliaryEffectSlotfv);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalLoaderImpl::load_essential_symbols()
try
{
	find_symbol("alcGetProcAddress", alcGetProcAddress_);
	find_symbol("alGetProcAddress", alGetProcAddress_);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalLoaderImpl::open_internal(const char* shared_library_path)
{
	shared_library_.open(shared_library_path);
	load_essential_symbols();
}

// =====================================

OalLoaderUPtr make_oal_loader(const char* shared_library_path)
{
	return std::make_unique<OalLoaderImpl>(shared_library_path);
}

} // namespace bstone
