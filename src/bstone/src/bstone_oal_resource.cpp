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

OalDeviceDeleter::OalDeviceDeleter(const OalDeviceDeleter& rhs)
	:
	al_symbols_{rhs.al_symbols_}
{}

OalDeviceDeleter::OalDeviceDeleter(const OalAlSymbols& al_symbols)
	:
	al_symbols_{&al_symbols}
{
	BSTONE_ASSERT(al_symbols_->alcCloseDevice != nullptr);
}

void OalDeviceDeleter::operator=(OalDeviceDeleter&& rhs) noexcept
{
	al_symbols_ = rhs.al_symbols_;
}

void OalDeviceDeleter::operator()(ALCdevice* alc_device) const
{
	BSTONE_ASSERT(alc_device != nullptr);
	BSTONE_ASSERT(al_symbols_ != nullptr);
	BSTONE_ASSERT(al_symbols_->alcCloseDevice != nullptr);
	[[maybe_unused]] const ALCboolean alc_result = al_symbols_->alcCloseDevice(alc_device);
	BSTONE_ASSERT(alc_result == ALC_TRUE);
}

OalDeviceResource make_oal_device(const OalAlSymbols& al_symbols, const char* device_name)
try
{
	if (al_symbols.alcOpenDevice == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alcOpenDevice\".");
	ALCdevice* const al_device = al_symbols.alcOpenDevice(device_name);
	if (al_device == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Failed to open a device.");
	return OalDeviceResource{al_device, OalDeviceDeleter{al_symbols}};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

OalContextDeleter::OalContextDeleter(const OalContextDeleter& rhs)
	:
	al_symbols_{rhs.al_symbols_}
{}

OalContextDeleter::OalContextDeleter(const OalAlSymbols& al_symbols)
	:
	al_symbols_{&al_symbols}
{
	BSTONE_ASSERT(al_symbols_ != nullptr);
	BSTONE_ASSERT(al_symbols_->alcGetCurrentContext != nullptr);
	BSTONE_ASSERT(al_symbols_->alcMakeContextCurrent != nullptr);
	BSTONE_ASSERT(al_symbols_->alcDestroyContext != nullptr);
}

void OalContextDeleter::operator=(OalContextDeleter&& rhs) noexcept
{
	al_symbols_ = rhs.al_symbols_;
}

void OalContextDeleter::operator()(ALCcontext* al_context) const
{
	BSTONE_ASSERT(al_context != nullptr);
	BSTONE_ASSERT(al_symbols_ != nullptr);
	BSTONE_ASSERT(al_symbols_->alcMakeContextCurrent != nullptr);
	BSTONE_ASSERT(al_symbols_->alcDestroyContext != nullptr);
	[[maybe_unused]] const ALCboolean make_current_result = al_symbols_->alcMakeContextCurrent(nullptr);
	BSTONE_ASSERT(make_current_result == ALC_TRUE);
	al_symbols_->alcDestroyContext(al_context);
}

OalContextResource make_oal_context(const OalAlSymbols& al_symbols, ALCdevice& al_device, const ALCint* al_context_attributes)
try
{
	if (al_symbols.alcCreateContext == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alcCreateContext\".");
	ALCcontext* const al_context = al_symbols.alcCreateContext(&al_device, al_context_attributes);
	if (al_context == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Failed to create a context.");
	return OalContextResource{al_context, OalContextDeleter{al_symbols}};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

OalBufferDeleter::OalBufferDeleter(const OalBufferDeleter& rhs)
	:
	al_symbols_{rhs.al_symbols_}
{}

OalBufferDeleter::OalBufferDeleter(const OalAlSymbols& al_symbols)
	:
	al_symbols_{&al_symbols}
{
	BSTONE_ASSERT(al_symbols_->alDeleteBuffers != nullptr);
}

void OalBufferDeleter::operator=(OalBufferDeleter&& rhs) noexcept
{
	al_symbols_ = rhs.al_symbols_;
}

void OalBufferDeleter::operator()(ALuint al_buffer) const
{
	BSTONE_ASSERT(al_buffer != AL_NONE);
	BSTONE_ASSERT(al_symbols_ != nullptr);
	BSTONE_ASSERT(al_symbols_->alDeleteBuffers != nullptr);
	al_symbols_->alDeleteBuffers(1, &al_buffer);
}

OalBufferResource make_oal_buffer(const OalAlSymbols& al_symbols)
try
{
	if (al_symbols.alGenBuffers == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alGenBuffers\".");
	if (!al_symbols.alIsBuffer)
		BSTONE_THROW_STATIC_SOURCE("Null \"alIsBuffer\".");
	ALuint al_buffer = 0;
	al_symbols.alGenBuffers(1, &al_buffer);
	if (const bool is_al_buffer = (al_symbols.alIsBuffer(al_buffer) == AL_TRUE);
		!is_al_buffer)
		BSTONE_THROW_STATIC_SOURCE("Failed to create a buffer.");
	return OalBufferResource{al_buffer, OalBufferDeleter{al_symbols}};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

OalSourceDeleter::OalSourceDeleter(const OalSourceDeleter& rhs)
	:
	al_symbols_{rhs.al_symbols_}
{}

OalSourceDeleter::OalSourceDeleter(const OalAlSymbols& al_symbols)
	:
	al_symbols_{&al_symbols}
{
	BSTONE_ASSERT(al_symbols_->alDeleteSources != nullptr);
}

void OalSourceDeleter::operator=(OalSourceDeleter&& rhs) noexcept
{
	al_symbols_ = rhs.al_symbols_;
}

void OalSourceDeleter::operator()(ALuint al_source) const
{
	BSTONE_ASSERT(al_source != AL_NONE);
	BSTONE_ASSERT(al_symbols_ != nullptr);
	BSTONE_ASSERT(al_symbols_->alDeleteSources != nullptr);
	al_symbols_->alDeleteSources(1, &al_source);
}

OalSourceResource make_oal_source(const OalAlSymbols& al_symbols)
try
{
	if (al_symbols.alGenSources == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Null \"alGenSources\".");
	if (!al_symbols.alIsSource)
		BSTONE_THROW_STATIC_SOURCE("Null \"alIsSource\".");
	ALuint al_source = 0;
	al_symbols.alGenSources(1, &al_source);
	if (const bool is_al_source = (al_symbols.alIsSource(al_source) == AL_TRUE);
		!is_al_source)
		BSTONE_THROW_STATIC_SOURCE("Failed to create a source.");
	return OalSourceResource{al_source, OalSourceDeleter{al_symbols}};
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
