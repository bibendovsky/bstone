/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_oal_resource.h"
#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_oal_symbols.h"

namespace bstone {

void OalDeviceDeleter::operator()(ALCdevice* alc_device) const
{
	if (alc_device == nullptr)
		return;
	BSTONE_ASSERT(alcCloseDevice != nullptr);
	[[maybe_unused]] const ALCboolean is_closed = alcCloseDevice(alc_device);
	BSTONE_ASSERT(is_closed == ALC_TRUE);
}

OalDeviceResource make_oal_device(const char* device_name)
try
{
	BSTONE_ASSERT(alcOpenDevice != nullptr);
	ALCdevice* const al_device = alcOpenDevice(device_name);
	if (al_device == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Failed to open a device.");
	return OalDeviceResource{al_device};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void OalContextDeleter::operator()(ALCcontext* al_context) const
{
	if (al_context == nullptr)
		return;
	BSTONE_ASSERT(alcMakeContextCurrent != nullptr);
	BSTONE_ASSERT(alcDestroyContext != nullptr);
	[[maybe_unused]] const ALCboolean is_made = alcMakeContextCurrent(nullptr);
	BSTONE_ASSERT(is_made == ALC_TRUE);
	alcDestroyContext(al_context);
}

OalContextResource make_oal_context(ALCdevice& al_device, const ALCint* al_context_attributes)
try
{
	BSTONE_ASSERT(alcCreateContext != nullptr);
	ALCcontext* const al_context = alcCreateContext(&al_device, al_context_attributes);
	if (al_context == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Failed to create a context.");
	return OalContextResource{al_context};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void OalBufferDeleter::operator()(ALuint al_buffer) const
{
	if (al_buffer == 0)
		return;
	BSTONE_ASSERT(alDeleteBuffers != nullptr);
	BSTONE_ASSERT(alIsBuffer != nullptr);
	alDeleteBuffers(1, &al_buffer);
	BSTONE_ASSERT(alIsBuffer(al_buffer) == AL_FALSE);
}

OalBufferResource make_oal_buffer()
try
{
	BSTONE_ASSERT(alGenBuffers != nullptr);
	BSTONE_ASSERT(alIsBuffer != nullptr);
	ALuint al_buffer = 0;
	alGenBuffers(1, &al_buffer);
	if (al_buffer == 0)
		BSTONE_THROW_STATIC_SOURCE("Failed to create a buffer.");
	return OalBufferResource{al_buffer};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void OalSourceDeleter::operator()(ALuint al_source) const
{
	if (al_source == 0)
		return;
	BSTONE_ASSERT(alDeleteSources != nullptr);
	BSTONE_ASSERT(alIsSource != nullptr);
	alDeleteSources(1, &al_source);
	BSTONE_ASSERT(alIsSource(al_source) == AL_FALSE);
}

OalSourceResource make_oal_source()
try
{
	BSTONE_ASSERT(alGenSources != nullptr);
	ALuint al_source = 0;
	alGenSources(1, &al_source);
	if (al_source == 0)
		BSTONE_THROW_STATIC_SOURCE("Failed to create a source.");
	return OalSourceResource{al_source};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
