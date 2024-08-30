/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_oal_loader.h"

#include <exception>
#include <string>
#include <utility>
#include "bstone_assert.h"
#include "bstone_shared_library.h"
#include "bstone_exception.h"

namespace bstone
{

OalLoader::OalLoader() noexcept = default;

OalLoader::~OalLoader() = default;

// ==========================================================================

class OalLoaderImpl final : public OalLoader
{
public:
	OalLoaderImpl(const char* shared_library_path);
	~OalLoaderImpl() override;

	void load_alc_symbols(OalAlSymbols& al_symbols) override;
	void load_al_symbols(OalAlSymbols& al_symbols) override;
	void load_efx_symbols(OalAlSymbols& al_symbols) override;

private:
	SharedLibrary shared_library_{};

	LPALCGETPROCADDRESS alcGetProcAddress_{};
	LPALGETPROCADDRESS alGetProcAddress_{};

	void load_essential_symbols();
	void open_internal(const char* path);

	template<typename T>
	void find_symbol(const char* name, T& symbol)
	{
		BSTONE_ASSERT(name && (*name) != '\0');

		symbol = shared_library_.find_symbol<T>(name);

		if (!symbol)
		{
			const auto message = std::string{} + "Symbol \"" + name + "\" not found.";
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
		BSTONE_ASSERT(name && (*name) != '\0');
		BSTONE_ASSERT(acronym && (*acronym) != '\0');
		BSTONE_ASSERT(func);

		symbol = reinterpret_cast<TSymbol>(func(std::forward<TFuncArgs>(func_args)...));

		if (!symbol)
		{
			const auto message = std::string{} + acronym + " symbol \"" + name + "\" not found.";
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
}; // OalLoaderImpl

// ==========================================================================

OalLoaderImpl::OalLoaderImpl(const char* shared_library_path)
{
	open_internal(shared_library_path);
}

OalLoaderImpl::~OalLoaderImpl() = default;

void OalLoaderImpl::load_alc_symbols(OalAlSymbols& al_symbols)
try {
	find_alc_symbol("alcCreateContext", al_symbols.alcCreateContext);
	find_alc_symbol("alcMakeContextCurrent", al_symbols.alcMakeContextCurrent);
	find_alc_symbol("alcProcessContext", al_symbols.alcProcessContext);
	find_alc_symbol("alcSuspendContext", al_symbols.alcSuspendContext);
	find_alc_symbol("alcDestroyContext", al_symbols.alcDestroyContext);
	find_alc_symbol("alcGetCurrentContext", al_symbols.alcGetCurrentContext);
	find_alc_symbol("alcGetContextsDevice", al_symbols.alcGetContextsDevice);
	find_alc_symbol("alcOpenDevice", al_symbols.alcOpenDevice);
	find_alc_symbol("alcCloseDevice", al_symbols.alcCloseDevice);
	find_alc_symbol("alcGetError", al_symbols.alcGetError);
	find_alc_symbol("alcIsExtensionPresent", al_symbols.alcIsExtensionPresent);
	find_alc_symbol("alcGetProcAddress", al_symbols.alcGetProcAddress);
	find_alc_symbol("alcGetEnumValue", al_symbols.alcGetEnumValue);
	find_alc_symbol("alcGetString", al_symbols.alcGetString);
	find_alc_symbol("alcGetIntegerv", al_symbols.alcGetIntegerv);
	find_alc_symbol("alcCaptureOpenDevice", al_symbols.alcCaptureOpenDevice);
	find_alc_symbol("alcCaptureCloseDevice", al_symbols.alcCaptureCloseDevice);
	find_alc_symbol("alcCaptureStart", al_symbols.alcCaptureStart);
	find_alc_symbol("alcCaptureStop", al_symbols.alcCaptureStop);
	find_alc_symbol("alcCaptureSamples", al_symbols.alcCaptureSamples);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalLoaderImpl::load_al_symbols(OalAlSymbols& al_symbols)
try {
	find_al_symbol("alDopplerFactor", al_symbols.alDopplerFactor);
	find_al_symbol("alDopplerVelocity", al_symbols.alDopplerVelocity);
	find_al_symbol("alSpeedOfSound", al_symbols.alSpeedOfSound);
	find_al_symbol("alDistanceModel", al_symbols.alDistanceModel);
	find_al_symbol("alEnable", al_symbols.alEnable);
	find_al_symbol("alDisable", al_symbols.alDisable);
	find_al_symbol("alIsEnabled", al_symbols.alIsEnabled);
	find_al_symbol("alGetString", al_symbols.alGetString);
	find_al_symbol("alGetBooleanv", al_symbols.alGetBooleanv);
	find_al_symbol("alGetIntegerv", al_symbols.alGetIntegerv);
	find_al_symbol("alGetFloatv", al_symbols.alGetFloatv);
	find_al_symbol("alGetDoublev", al_symbols.alGetDoublev);
	find_al_symbol("alGetBoolean", al_symbols.alGetBoolean);
	find_al_symbol("alGetInteger", al_symbols.alGetInteger);
	find_al_symbol("alGetFloat", al_symbols.alGetFloat);
	find_al_symbol("alGetDouble", al_symbols.alGetDouble);
	find_al_symbol("alGetError", al_symbols.alGetError);
	find_al_symbol("alIsExtensionPresent", al_symbols.alIsExtensionPresent);
	find_al_symbol("alGetProcAddress", al_symbols.alGetProcAddress);
	find_al_symbol("alGetEnumValue", al_symbols.alGetEnumValue);
	find_al_symbol("alListenerf", al_symbols.alListenerf);
	find_al_symbol("alListener3f", al_symbols.alListener3f);
	find_al_symbol("alListenerfv", al_symbols.alListenerfv);
	find_al_symbol("alListeneri", al_symbols.alListeneri);
	find_al_symbol("alListener3i", al_symbols.alListener3i);
	find_al_symbol("alListeneriv", al_symbols.alListeneriv);
	find_al_symbol("alGetListenerf", al_symbols.alGetListenerf);
	find_al_symbol("alGetListener3f", al_symbols.alGetListener3f);
	find_al_symbol("alGetListenerfv", al_symbols.alGetListenerfv);
	find_al_symbol("alGetListeneri", al_symbols.alGetListeneri);
	find_al_symbol("alGetListener3i", al_symbols.alGetListener3i);
	find_al_symbol("alGetListeneriv", al_symbols.alGetListeneriv);
	find_al_symbol("alGenSources", al_symbols.alGenSources);
	find_al_symbol("alDeleteSources", al_symbols.alDeleteSources);
	find_al_symbol("alIsSource", al_symbols.alIsSource);
	find_al_symbol("alSourcef", al_symbols.alSourcef);
	find_al_symbol("alSource3f", al_symbols.alSource3f);
	find_al_symbol("alSourcefv", al_symbols.alSourcefv);
	find_al_symbol("alSourcei", al_symbols.alSourcei);
	find_al_symbol("alSource3i", al_symbols.alSource3i);
	find_al_symbol("alSourceiv", al_symbols.alSourceiv);
	find_al_symbol("alGetSourcef", al_symbols.alGetSourcef);
	find_al_symbol("alGetSource3f", al_symbols.alGetSource3f);
	find_al_symbol("alGetSourcefv", al_symbols.alGetSourcefv);
	find_al_symbol("alGetSourcei", al_symbols.alGetSourcei);
	find_al_symbol("alGetSource3i", al_symbols.alGetSource3i);
	find_al_symbol("alGetSourceiv", al_symbols.alGetSourceiv);
	find_al_symbol("alSourcePlayv", al_symbols.alSourcePlayv);
	find_al_symbol("alSourceStopv", al_symbols.alSourceStopv);
	find_al_symbol("alSourceRewindv", al_symbols.alSourceRewindv);
	find_al_symbol("alSourcePausev", al_symbols.alSourcePausev);
	find_al_symbol("alSourcePlay", al_symbols.alSourcePlay);
	find_al_symbol("alSourceStop", al_symbols.alSourceStop);
	find_al_symbol("alSourceRewind", al_symbols.alSourceRewind);
	find_al_symbol("alSourcePause", al_symbols.alSourcePause);
	find_al_symbol("alSourceQueueBuffers", al_symbols.alSourceQueueBuffers);
	find_al_symbol("alSourceUnqueueBuffers", al_symbols.alSourceUnqueueBuffers);
	find_al_symbol("alGenBuffers", al_symbols.alGenBuffers);
	find_al_symbol("alDeleteBuffers", al_symbols.alDeleteBuffers);
	find_al_symbol("alIsBuffer", al_symbols.alIsBuffer);
	find_al_symbol("alBufferData", al_symbols.alBufferData);
	find_al_symbol("alBufferf", al_symbols.alBufferf);
	find_al_symbol("alBuffer3f", al_symbols.alBuffer3f);
	find_al_symbol("alBufferfv", al_symbols.alBufferfv);
	find_al_symbol("alBufferi", al_symbols.alBufferi);
	find_al_symbol("alBuffer3i", al_symbols.alBuffer3i);
	find_al_symbol("alBufferiv", al_symbols.alBufferiv);
	find_al_symbol("alGetBufferf", al_symbols.alGetBufferf);
	find_al_symbol("alGetBuffer3f", al_symbols.alGetBuffer3f);
	find_al_symbol("alGetBufferfv", al_symbols.alGetBufferfv);
	find_al_symbol("alGetBufferi", al_symbols.alGetBufferi);
	find_al_symbol("alGetBuffer3i", al_symbols.alGetBuffer3i);
	find_al_symbol("alGetBufferiv", al_symbols.alGetBufferiv);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalLoaderImpl::load_efx_symbols(OalAlSymbols& al_symbols)
try {
	find_al_symbol("alGenEffects", al_symbols.alGenEffects);
	find_al_symbol("alDeleteEffects", al_symbols.alDeleteEffects);
	find_al_symbol("alIsEffect", al_symbols.alIsEffect);
	find_al_symbol("alEffecti", al_symbols.alEffecti);
	find_al_symbol("alEffectiv", al_symbols.alEffectiv);
	find_al_symbol("alEffectf", al_symbols.alEffectf);
	find_al_symbol("alEffectfv", al_symbols.alEffectfv);
	find_al_symbol("alGetEffecti", al_symbols.alGetEffecti);
	find_al_symbol("alGetEffectiv", al_symbols.alGetEffectiv);
	find_al_symbol("alGetEffectf", al_symbols.alGetEffectf);
	find_al_symbol("alGetEffectfv", al_symbols.alGetEffectfv);
	find_al_symbol("alGenFilters", al_symbols.alGenFilters);
	find_al_symbol("alDeleteFilters", al_symbols.alDeleteFilters);
	find_al_symbol("alIsFilter", al_symbols.alIsFilter);
	find_al_symbol("alFilteri", al_symbols.alFilteri);
	find_al_symbol("alFilteriv", al_symbols.alFilteriv);
	find_al_symbol("alFilterf", al_symbols.alFilterf);
	find_al_symbol("alFilterfv", al_symbols.alFilterfv);
	find_al_symbol("alGetFilteri", al_symbols.alGetFilteri);
	find_al_symbol("alGetFilteriv", al_symbols.alGetFilteriv);
	find_al_symbol("alGetFilterf", al_symbols.alGetFilterf);
	find_al_symbol("alGetFilterfv", al_symbols.alGetFilterfv);
	find_al_symbol("alGenAuxiliaryEffectSlots", al_symbols.alGenAuxiliaryEffectSlots);
	find_al_symbol("alDeleteAuxiliaryEffectSlots", al_symbols.alDeleteAuxiliaryEffectSlots);
	find_al_symbol("alIsAuxiliaryEffectSlot", al_symbols.alIsAuxiliaryEffectSlot);
	find_al_symbol("alAuxiliaryEffectSloti", al_symbols.alAuxiliaryEffectSloti);
	find_al_symbol("alAuxiliaryEffectSlotiv", al_symbols.alAuxiliaryEffectSlotiv);
	find_al_symbol("alAuxiliaryEffectSlotf", al_symbols.alAuxiliaryEffectSlotf);
	find_al_symbol("alAuxiliaryEffectSlotfv", al_symbols.alAuxiliaryEffectSlotfv);
	find_al_symbol("alGetAuxiliaryEffectSloti", al_symbols.alGetAuxiliaryEffectSloti);
	find_al_symbol("alGetAuxiliaryEffectSlotiv", al_symbols.alGetAuxiliaryEffectSlotiv);
	find_al_symbol("alGetAuxiliaryEffectSlotf", al_symbols.alGetAuxiliaryEffectSlotf);
	find_al_symbol("alGetAuxiliaryEffectSlotfv", al_symbols.alGetAuxiliaryEffectSlotfv);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalLoaderImpl::load_essential_symbols()
try {
	find_symbol("alcGetProcAddress", alcGetProcAddress_);
	find_symbol("alGetProcAddress", alGetProcAddress_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void OalLoaderImpl::open_internal(const char* shared_library_path)
{
	shared_library_.open(shared_library_path);
	load_essential_symbols();
}

// ==========================================================================

OalLoaderUPtr make_oal_loader(const char* shared_library_path)
{
	return std::make_unique<OalLoaderImpl>(shared_library_path);
}

} // bstone
