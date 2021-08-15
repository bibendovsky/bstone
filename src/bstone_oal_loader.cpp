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


#include "bstone_oal_loader.h"

#include <cassert>

#include <exception>
#include <string>
#include <utility>

#include "bstone_shared_library.h"
#include "bstone_exception.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalLoaderException :
	public Exception
{
public:
	explicit OalLoaderException(
		const char* message) noexcept
		:
		Exception{"OAL_LOADER", message}
	{
	}
}; // OalLoaderException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalLoaderImpl final :
	public OalLoader
{
public:
	OalLoaderImpl(
		const char* path);


	OalAlcSymbols load_alc_symbols() override;

	OalAlSymbols load_al_symbols() override;


private:
	SharedLibraryUPtr shared_library_{};

	::LPALCGETPROCADDRESS alcGetProcAddress_{};
	::LPALCGETCURRENTCONTEXT alcGetCurrentContext_{};
	::LPALGETPROCADDRESS alGetProcAddress_{};


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);

	void load_essential_symbols();

	void open_internal(
		const char* path);


	template<
		typename T
	>
	void find_symbol(
		const char* name,
		T& symbol)
	{
		if (!name || (*name) == '\0')
		{
			fail("Null or empty symbol name.");
		}

		symbol = reinterpret_cast<T>(shared_library_->find_symbol(name));

		if (!symbol)
		{
			const auto message = std::string{} + "Symbol \"" + name + "\" not found.";

			fail(message.c_str());
		}
	}

	template<
		typename TSymbol,
		typename TFuncResult,
		typename ...TFuncArgs,
		typename ...TArgs
	>
	void find_alx_symbol(
		const char* name,
		TSymbol& symbol,
		const char* acronym,
		TFuncResult (*func)(TFuncArgs...),
		TArgs... func_args)
	{
		assert(acronym && (*acronym) != '\0');

		if (!func)
		{
			const auto message = std::string{} + "Null " + acronym + " function.";
			fail(message.c_str());
		}

		if (!name || (*name) == '\0')
		{
			const auto message = std::string{} + "Null or empty " + acronym + " symbol name.";
			fail(message.c_str());
		}

		symbol = reinterpret_cast<TSymbol>(func(std::forward<TFuncArgs>(func_args)...));

		if (!symbol)
		{
			const auto message = std::string{} + acronym + " symbol \"" + name + "\" not found.";
			fail(message.c_str());
		}
	}

	template<
		typename TSymbol
	>
	void find_alc_symbol(
		const char* name,
		TSymbol& symbol)
	{
		find_alx_symbol(
			name,
			symbol,
			"ALC",
			alcGetProcAddress_,
			nullptr,
			name
		);
	}

	template<
		typename TSymbol
	>
	void find_al_symbol(
		const char* name,
		TSymbol& symbol)
	{
		find_alx_symbol(
			name,
			symbol,
			"AL",
			alGetProcAddress_,
			name
		);
	}
}; // OalLoaderImpl

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalLoaderImpl::OalLoaderImpl(
	const char* path)
{
	open_internal(path);
}

OalAlcSymbols OalLoaderImpl::load_alc_symbols()
try
{
	auto oal_alc_symbols = OalAlcSymbols{};

	find_alc_symbol("alcCreateContext", oal_alc_symbols.alcCreateContext);
	find_alc_symbol("alcMakeContextCurrent", oal_alc_symbols.alcMakeContextCurrent);
	find_alc_symbol("alcProcessContext", oal_alc_symbols.alcProcessContext);
	find_alc_symbol("alcSuspendContext", oal_alc_symbols.alcSuspendContext);
	find_alc_symbol("alcDestroyContext", oal_alc_symbols.alcDestroyContext);
	find_alc_symbol("alcGetCurrentContext", oal_alc_symbols.alcGetCurrentContext);
	find_alc_symbol("alcGetContextsDevice", oal_alc_symbols.alcGetContextsDevice);
	find_alc_symbol("alcOpenDevice", oal_alc_symbols.alcOpenDevice);
	find_alc_symbol("alcCloseDevice", oal_alc_symbols.alcCloseDevice);
	find_alc_symbol("alcGetError", oal_alc_symbols.alcGetError);
	find_alc_symbol("alcIsExtensionPresent", oal_alc_symbols.alcIsExtensionPresent);
	find_alc_symbol("alcGetProcAddress", oal_alc_symbols.alcGetProcAddress);
	find_alc_symbol("alcGetEnumValue", oal_alc_symbols.alcGetEnumValue);
	find_alc_symbol("alcGetString", oal_alc_symbols.alcGetString);
	find_alc_symbol("alcGetIntegerv", oal_alc_symbols.alcGetIntegerv);
	find_alc_symbol("alcCaptureOpenDevice", oal_alc_symbols.alcCaptureOpenDevice);
	find_alc_symbol("alcCaptureCloseDevice", oal_alc_symbols.alcCaptureCloseDevice);
	find_alc_symbol("alcCaptureStart", oal_alc_symbols.alcCaptureStart);
	find_alc_symbol("alcCaptureStop", oal_alc_symbols.alcCaptureStop);
	find_alc_symbol("alcCaptureSamples", oal_alc_symbols.alcCaptureSamples);

	return oal_alc_symbols;
}
catch (...)
{
	fail_nested("Failed to load ALC symbols.");
}

OalAlSymbols OalLoaderImpl::load_al_symbols()
try
{
	const auto alc_context = alcGetCurrentContext_();

	if (!alc_context)
	{
		fail("No current context.");
	}

	auto oal_al_symbols = OalAlSymbols{};

	find_al_symbol("alDopplerFactor", oal_al_symbols.alDopplerFactor);
	find_al_symbol("alDopplerVelocity", oal_al_symbols.alDopplerVelocity);
	find_al_symbol("alSpeedOfSound", oal_al_symbols.alSpeedOfSound);
	find_al_symbol("alDistanceModel", oal_al_symbols.alDistanceModel);
	find_al_symbol("alEnable", oal_al_symbols.alEnable);
	find_al_symbol("alDisable", oal_al_symbols.alDisable);
	find_al_symbol("alIsEnabled", oal_al_symbols.alIsEnabled);
	find_al_symbol("alGetString", oal_al_symbols.alGetString);
	find_al_symbol("alGetBooleanv", oal_al_symbols.alGetBooleanv);
	find_al_symbol("alGetIntegerv", oal_al_symbols.alGetIntegerv);
	find_al_symbol("alGetFloatv", oal_al_symbols.alGetFloatv);
	find_al_symbol("alGetDoublev", oal_al_symbols.alGetDoublev);
	find_al_symbol("alGetBoolean", oal_al_symbols.alGetBoolean);
	find_al_symbol("alGetInteger", oal_al_symbols.alGetInteger);
	find_al_symbol("alGetFloat", oal_al_symbols.alGetFloat);
	find_al_symbol("alGetDouble", oal_al_symbols.alGetDouble);
	find_al_symbol("alGetError", oal_al_symbols.alGetError);
	find_al_symbol("alIsExtensionPresent", oal_al_symbols.alIsExtensionPresent);
	find_al_symbol("alGetProcAddress", oal_al_symbols.alGetProcAddress);
	find_al_symbol("alGetEnumValue", oal_al_symbols.alGetEnumValue);
	find_al_symbol("alListenerf", oal_al_symbols.alListenerf);
	find_al_symbol("alListener3f", oal_al_symbols.alListener3f);
	find_al_symbol("alListenerfv", oal_al_symbols.alListenerfv);
	find_al_symbol("alListeneri", oal_al_symbols.alListeneri);
	find_al_symbol("alListener3i", oal_al_symbols.alListener3i);
	find_al_symbol("alListeneriv", oal_al_symbols.alListeneriv);
	find_al_symbol("alGetListenerf", oal_al_symbols.alGetListenerf);
	find_al_symbol("alGetListener3f", oal_al_symbols.alGetListener3f);
	find_al_symbol("alGetListenerfv", oal_al_symbols.alGetListenerfv);
	find_al_symbol("alGetListeneri", oal_al_symbols.alGetListeneri);
	find_al_symbol("alGetListener3i", oal_al_symbols.alGetListener3i);
	find_al_symbol("alGetListeneriv", oal_al_symbols.alGetListeneriv);
	find_al_symbol("alGenSources", oal_al_symbols.alGenSources);
	find_al_symbol("alDeleteSources", oal_al_symbols.alDeleteSources);
	find_al_symbol("alIsSource", oal_al_symbols.alIsSource);
	find_al_symbol("alSourcef", oal_al_symbols.alSourcef);
	find_al_symbol("alSource3f", oal_al_symbols.alSource3f);
	find_al_symbol("alSourcefv", oal_al_symbols.alSourcefv);
	find_al_symbol("alSourcei", oal_al_symbols.alSourcei);
	find_al_symbol("alSource3i", oal_al_symbols.alSource3i);
	find_al_symbol("alSourceiv", oal_al_symbols.alSourceiv);
	find_al_symbol("alGetSourcef", oal_al_symbols.alGetSourcef);
	find_al_symbol("alGetSource3f", oal_al_symbols.alGetSource3f);
	find_al_symbol("alGetSourcefv", oal_al_symbols.alGetSourcefv);
	find_al_symbol("alGetSourcei", oal_al_symbols.alGetSourcei);
	find_al_symbol("alGetSource3i", oal_al_symbols.alGetSource3i);
	find_al_symbol("alGetSourceiv", oal_al_symbols.alGetSourceiv);
	find_al_symbol("alSourcePlayv", oal_al_symbols.alSourcePlayv);
	find_al_symbol("alSourceStopv", oal_al_symbols.alSourceStopv);
	find_al_symbol("alSourceRewindv", oal_al_symbols.alSourceRewindv);
	find_al_symbol("alSourcePausev", oal_al_symbols.alSourcePausev);
	find_al_symbol("alSourcePlay", oal_al_symbols.alSourcePlay);
	find_al_symbol("alSourceStop", oal_al_symbols.alSourceStop);
	find_al_symbol("alSourceRewind", oal_al_symbols.alSourceRewind);
	find_al_symbol("alSourcePause", oal_al_symbols.alSourcePause);
	find_al_symbol("alSourceQueueBuffers", oal_al_symbols.alSourceQueueBuffers);
	find_al_symbol("alSourceUnqueueBuffers", oal_al_symbols.alSourceUnqueueBuffers);
	find_al_symbol("alGenBuffers", oal_al_symbols.alGenBuffers);
	find_al_symbol("alDeleteBuffers", oal_al_symbols.alDeleteBuffers);
	find_al_symbol("alIsBuffer", oal_al_symbols.alIsBuffer);
	find_al_symbol("alBufferData", oal_al_symbols.alBufferData);
	find_al_symbol("alBufferf", oal_al_symbols.alBufferf);
	find_al_symbol("alBuffer3f", oal_al_symbols.alBuffer3f);
	find_al_symbol("alBufferfv", oal_al_symbols.alBufferfv);
	find_al_symbol("alBufferi", oal_al_symbols.alBufferi);
	find_al_symbol("alBuffer3i", oal_al_symbols.alBuffer3i);
	find_al_symbol("alBufferiv", oal_al_symbols.alBufferiv);
	find_al_symbol("alGetBufferf", oal_al_symbols.alGetBufferf);
	find_al_symbol("alGetBuffer3f", oal_al_symbols.alGetBuffer3f);
	find_al_symbol("alGetBufferfv", oal_al_symbols.alGetBufferfv);
	find_al_symbol("alGetBufferi", oal_al_symbols.alGetBufferi);
	find_al_symbol("alGetBuffer3i", oal_al_symbols.alGetBuffer3i);
	find_al_symbol("alGetBufferiv", oal_al_symbols.alGetBufferiv);

	return oal_al_symbols;
}
catch (...)
{
	fail_nested("Failed to load AL symbols.");
}

[[noreturn]]
void OalLoaderImpl::fail(
	const char* message)
{
	throw OalLoaderException{message};
}

[[noreturn]]
void OalLoaderImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(OalLoaderException{message});
}

void OalLoaderImpl::load_essential_symbols()
try
{
	find_symbol("alcGetProcAddress", alcGetProcAddress_);
	find_alc_symbol("alcGetCurrentContext", alcGetCurrentContext_);

	find_symbol("alGetProcAddress", alGetProcAddress_);
}
catch (...)
{
	fail_nested("Failed to load essential symbols.");
}

void OalLoaderImpl::open_internal(
	const char* path)
{
	shared_library_ = make_shared_library(path);

	load_essential_symbols();
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalLoaderUPtr make_oal_loader(
	const char* path)
{
	return std::make_unique<OalLoaderImpl>(path);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
