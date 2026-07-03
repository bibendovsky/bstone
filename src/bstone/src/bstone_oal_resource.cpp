/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_oal_resource.h"
#include "bstone_assert.h"
#include "bstone_exception.h"
#include <tuple>
#include <utility>

namespace bstone {

void OalDeviceDeleter::operator()(ALCdevice* alc_device) const
{
	BSTONE_ASSERT(alc_device != nullptr);
	BSTONE_ASSERT(alcCloseDevice != nullptr);
	[[maybe_unused]] const ALCboolean alc_result = alcCloseDevice(alc_device);
	BSTONE_ASSERT(alc_result == ALC_TRUE);
}

OalDeviceResource make_oal_device(const char* device_name)
try
{
	if (alcOpenDevice == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alcOpenDevice\".");
	ALCdevice* const al_device = alcOpenDevice(device_name);
	if (al_device == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Failed to open a device.");
	return OalDeviceResource{al_device};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void OalContextDeleter::operator()(ALCcontext* al_context) const
{
	BSTONE_ASSERT(al_context != nullptr);
	BSTONE_ASSERT(alcMakeContextCurrent != nullptr);
	BSTONE_ASSERT(alcDestroyContext != nullptr);
	[[maybe_unused]] const ALCboolean make_current_result = alcMakeContextCurrent(nullptr);
	BSTONE_ASSERT(make_current_result == ALC_TRUE);
	alcDestroyContext(al_context);
}

OalContextResource make_oal_context(ALCdevice& al_device, const ALCint* al_context_attributes)
try
{
	if (alcCreateContext == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alcCreateContext\".");
	ALCcontext* const al_context = alcCreateContext(&al_device, al_context_attributes);
	if (al_context == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Failed to create a context.");
	return OalContextResource{al_context};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void OalBufferDeleter::operator()(ALuint al_buffer) const
{
	BSTONE_ASSERT(al_buffer != AL_NONE);
	BSTONE_ASSERT(alDeleteBuffers != nullptr);
	alDeleteBuffers(1, &al_buffer);
}

OalBufferResource make_oal_buffer()
try
{
	if (alGenBuffers == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alGenBuffers\".");
	if (alIsBuffer == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alIsBuffer\".");
	ALuint al_buffer = 0;
	alGenBuffers(1, &al_buffer);
	if (const bool is_al_buffer = (alIsBuffer(al_buffer) == AL_TRUE);
		!is_al_buffer)
		BSTONE_THROW_STATIC_SOURCE("Failed to create a buffer.");
	return OalBufferResource{al_buffer};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void OalSourceDeleter::operator()(ALuint al_source) const
{
	BSTONE_ASSERT(al_source != AL_NONE);
	BSTONE_ASSERT(alDeleteSources != nullptr);
	alDeleteSources(1, &al_source);
}

OalSourceResource make_oal_source()
try
{
	if (alGenSources == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alGenSources\".");
	if (alIsSource == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alIsSource\".");
	ALuint al_source = 0;
	alGenSources(1, &al_source);
	if (const bool is_al_source = (alIsSource(al_source) == AL_TRUE);
		!is_al_source)
		BSTONE_THROW_STATIC_SOURCE("Failed to create a source.");
	return OalSourceResource{al_source};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
