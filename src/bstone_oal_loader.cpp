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

#include <array>

#include "bstone_dynamic_loader.h"
#include "bstone_exception.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalLoader::OalLoader() noexcept = default;

OalLoader::~OalLoader() = default;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalLoaderImplException :
	public Exception
{
public:
	explicit OalLoaderImplException(
		const char* message)
		:
		Exception{std::string{"[OAL_LOADER_IMPL] "} + message}
	{
	}
}; // OalLoaderImplException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalLoaderImpl final :
	public OalLoader
{
public:
	OalLoaderImpl(
		const char* shared_library_path_name);


	const OalSymbols& get_symbols() const noexcept override;


private:
	using SharedLibraryPathNames = std::array<const char*, 2>;


	DynamicLoaderUPtr dynamic_loader_{};
	OalSymbols oal_symbols_{};


	void open_internal(
		const char* shared_library_path_name);


	template<
		typename T
	>
	void resolve_symbol(
		const char* name,
		T& symbol)
	{
		symbol = reinterpret_cast<T>(dynamic_loader_->resolve(name));

		if (symbol == nullptr)
		{
			throw OalLoaderImplException{name};
		}
	}
}; // OalLoaderImpl

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalLoaderImpl::OalLoaderImpl(
	const char* shared_library_path_name)
{
	open_internal(shared_library_path_name);
}

const OalSymbols& OalLoaderImpl::get_symbols() const noexcept
{
	return oal_symbols_;
}

void OalLoaderImpl::open_internal(
	const char* shared_library_path_name)
{
	dynamic_loader_ = make_dynamic_loader(shared_library_path_name);


	//
	// AL 1.1
	//

	resolve_symbol("alDopplerFactor", oal_symbols_.alDopplerFactor);
	resolve_symbol("alDopplerVelocity", oal_symbols_.alDopplerVelocity);
	resolve_symbol("alSpeedOfSound", oal_symbols_.alSpeedOfSound);
	resolve_symbol("alDistanceModel", oal_symbols_.alDistanceModel);
	resolve_symbol("alEnable", oal_symbols_.alEnable);
	resolve_symbol("alDisable", oal_symbols_.alDisable);
	resolve_symbol("alIsEnabled", oal_symbols_.alIsEnabled);
	resolve_symbol("alGetString", oal_symbols_.alGetString);
	resolve_symbol("alGetBooleanv", oal_symbols_.alGetBooleanv);
	resolve_symbol("alGetIntegerv", oal_symbols_.alGetIntegerv);
	resolve_symbol("alGetFloatv", oal_symbols_.alGetFloatv);
	resolve_symbol("alGetDoublev", oal_symbols_.alGetDoublev);
	resolve_symbol("alGetBoolean", oal_symbols_.alGetBoolean);
	resolve_symbol("alGetInteger", oal_symbols_.alGetInteger);
	resolve_symbol("alGetFloat", oal_symbols_.alGetFloat);
	resolve_symbol("alGetDouble", oal_symbols_.alGetDouble);
	resolve_symbol("alGetError", oal_symbols_.alGetError);
	resolve_symbol("alIsExtensionPresent", oal_symbols_.alIsExtensionPresent);
	resolve_symbol("alGetProcAddress", oal_symbols_.alGetProcAddress);
	resolve_symbol("alGetEnumValue", oal_symbols_.alGetEnumValue);
	resolve_symbol("alListenerf", oal_symbols_.alListenerf);
	resolve_symbol("alListener3f", oal_symbols_.alListener3f);
	resolve_symbol("alListenerfv", oal_symbols_.alListenerfv);
	resolve_symbol("alListeneri", oal_symbols_.alListeneri);
	resolve_symbol("alListener3i", oal_symbols_.alListener3i);
	resolve_symbol("alListeneriv", oal_symbols_.alListeneriv);
	resolve_symbol("alGetListenerf", oal_symbols_.alGetListenerf);
	resolve_symbol("alGetListener3f", oal_symbols_.alGetListener3f);
	resolve_symbol("alGetListenerfv", oal_symbols_.alGetListenerfv);
	resolve_symbol("alGetListeneri", oal_symbols_.alGetListeneri);
	resolve_symbol("alGetListener3i", oal_symbols_.alGetListener3i);
	resolve_symbol("alGetListeneriv", oal_symbols_.alGetListeneriv);
	resolve_symbol("alGenSources", oal_symbols_.alGenSources);
	resolve_symbol("alDeleteSources", oal_symbols_.alDeleteSources);
	resolve_symbol("alIsSource", oal_symbols_.alIsSource);
	resolve_symbol("alSourcef", oal_symbols_.alSourcef);
	resolve_symbol("alSource3f", oal_symbols_.alSource3f);
	resolve_symbol("alSourcefv", oal_symbols_.alSourcefv);
	resolve_symbol("alSourcei", oal_symbols_.alSourcei);
	resolve_symbol("alSource3i", oal_symbols_.alSource3i);
	resolve_symbol("alSourceiv", oal_symbols_.alSourceiv);
	resolve_symbol("alGetSourcef", oal_symbols_.alGetSourcef);
	resolve_symbol("alGetSource3f", oal_symbols_.alGetSource3f);
	resolve_symbol("alGetSourcefv", oal_symbols_.alGetSourcefv);
	resolve_symbol("alGetSourcei", oal_symbols_.alGetSourcei);
	resolve_symbol("alGetSource3i", oal_symbols_.alGetSource3i);
	resolve_symbol("alGetSourceiv", oal_symbols_.alGetSourceiv);
	resolve_symbol("alSourcePlayv", oal_symbols_.alSourcePlayv);
	resolve_symbol("alSourceStopv", oal_symbols_.alSourceStopv);
	resolve_symbol("alSourceRewindv", oal_symbols_.alSourceRewindv);
	resolve_symbol("alSourcePausev", oal_symbols_.alSourcePausev);
	resolve_symbol("alSourcePlay", oal_symbols_.alSourcePlay);
	resolve_symbol("alSourceStop", oal_symbols_.alSourceStop);
	resolve_symbol("alSourceRewind", oal_symbols_.alSourceRewind);
	resolve_symbol("alSourcePause", oal_symbols_.alSourcePause);
	resolve_symbol("alSourceQueueBuffers", oal_symbols_.alSourceQueueBuffers);
	resolve_symbol("alSourceUnqueueBuffers", oal_symbols_.alSourceUnqueueBuffers);
	resolve_symbol("alGenBuffers", oal_symbols_.alGenBuffers);
	resolve_symbol("alDeleteBuffers", oal_symbols_.alDeleteBuffers);
	resolve_symbol("alIsBuffer", oal_symbols_.alIsBuffer);
	resolve_symbol("alBufferData", oal_symbols_.alBufferData);
	resolve_symbol("alBufferf", oal_symbols_.alBufferf);
	resolve_symbol("alBuffer3f", oal_symbols_.alBuffer3f);
	resolve_symbol("alBufferfv", oal_symbols_.alBufferfv);
	resolve_symbol("alBufferi", oal_symbols_.alBufferi);
	resolve_symbol("alBuffer3i", oal_symbols_.alBuffer3i);
	resolve_symbol("alBufferiv", oal_symbols_.alBufferiv);
	resolve_symbol("alGetBufferf", oal_symbols_.alGetBufferf);
	resolve_symbol("alGetBuffer3f", oal_symbols_.alGetBuffer3f);
	resolve_symbol("alGetBufferfv", oal_symbols_.alGetBufferfv);
	resolve_symbol("alGetBufferi", oal_symbols_.alGetBufferi);
	resolve_symbol("alGetBuffer3i", oal_symbols_.alGetBuffer3i);
	resolve_symbol("alGetBufferiv", oal_symbols_.alGetBufferiv);


	//
	// ALC 1_.1
	//

	resolve_symbol("alcCreateContext", oal_symbols_.alcCreateContext);
	resolve_symbol("alcMakeContextCurrent", oal_symbols_.alcMakeContextCurrent);
	resolve_symbol("alcProcessContext", oal_symbols_.alcProcessContext);
	resolve_symbol("alcSuspendContext", oal_symbols_.alcSuspendContext);
	resolve_symbol("alcDestroyContext", oal_symbols_.alcDestroyContext);
	resolve_symbol("alcGetCurrentContext", oal_symbols_.alcGetCurrentContext);
	resolve_symbol("alcGetContextsDevice", oal_symbols_.alcGetContextsDevice);
	resolve_symbol("alcOpenDevice", oal_symbols_.alcOpenDevice);
	resolve_symbol("alcCloseDevice", oal_symbols_.alcCloseDevice);
	resolve_symbol("alcGetError", oal_symbols_.alcGetError);
	resolve_symbol("alcIsExtensionPresent", oal_symbols_.alcIsExtensionPresent);
	resolve_symbol("alcGetProcAddress", oal_symbols_.alcGetProcAddress);
	resolve_symbol("alcGetEnumValue", oal_symbols_.alcGetEnumValue);
	resolve_symbol("alcGetString", oal_symbols_.alcGetString);
	resolve_symbol("alcGetIntegerv", oal_symbols_.alcGetIntegerv);
	resolve_symbol("alcCaptureOpenDevice", oal_symbols_.alcCaptureOpenDevice);
	resolve_symbol("alcCaptureCloseDevice", oal_symbols_.alcCaptureCloseDevice);
	resolve_symbol("alcCaptureStart", oal_symbols_.alcCaptureStart);
	resolve_symbol("alcCaptureStop", oal_symbols_.alcCaptureStop);
	resolve_symbol("alcCaptureSamples", oal_symbols_.alcCaptureSamples);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalLoaderUPtr make_oal_loader(
	const char* shared_library_path_name)
{
	return std::make_unique<OalLoaderImpl>(shared_library_path_name);
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
