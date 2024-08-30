/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_oal_resource.h"

#include <tuple>
#include <utility>
#include "bstone_assert.h"
#include "bstone_exception.h"


namespace bstone
{

OalDeviceDeleter::OalDeviceDeleter() noexcept = default;

OalDeviceDeleter::OalDeviceDeleter(const OalDeviceDeleter& rhs)
	:
	al_symbols_{rhs.al_symbols_}
{
}

OalDeviceDeleter::OalDeviceDeleter(const OalAlSymbols& al_symbols) noexcept
	:
	al_symbols_{&al_symbols}
{
	BSTONE_ASSERT(al_symbols_->alcCloseDevice);
}

void OalDeviceDeleter::operator=(OalDeviceDeleter&& rhs) noexcept
{
	al_symbols_ = rhs.al_symbols_;
}

void OalDeviceDeleter::operator()(ALCdevice* alc_device) const noexcept
{
	BSTONE_ASSERT(alc_device);
	BSTONE_ASSERT(al_symbols_);
	BSTONE_ASSERT(al_symbols_->alcCloseDevice);
	const auto alc_result = al_symbols_->alcCloseDevice(alc_device);
#ifdef NDEBUG
	std::ignore = alc_result;
#else
	BSTONE_ASSERT(alc_result == ALC_TRUE);
#endif // NDEBUG
}

OalDeviceResource make_oal_device(const OalAlSymbols& al_symbols, const char* device_name)
try {
	if (!al_symbols.alcOpenDevice)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"alcOpenDevice\".");
	}

	const auto al_device = al_symbols.alcOpenDevice(device_name);

	if (!al_device)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to open a device.");
	}

	return OalDeviceResource{al_device, OalDeviceDeleter{al_symbols}};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

OalContextDeleter::OalContextDeleter() noexcept = default;

OalContextDeleter::OalContextDeleter(const OalContextDeleter& rhs) noexcept
	:
	al_symbols_{rhs.al_symbols_}
{
}

OalContextDeleter::OalContextDeleter(const OalAlSymbols& al_symbols) noexcept
	:
	al_symbols_{&al_symbols}
{
	BSTONE_ASSERT(al_symbols_);
	BSTONE_ASSERT(al_symbols_->alcGetCurrentContext);
	BSTONE_ASSERT(al_symbols_->alcMakeContextCurrent);
	BSTONE_ASSERT(al_symbols_->alcDestroyContext);
}

void OalContextDeleter::operator=(OalContextDeleter&& rhs) noexcept
{
	al_symbols_ = rhs.al_symbols_;
}

void OalContextDeleter::operator()(ALCcontext* al_context) const noexcept
{
	BSTONE_ASSERT(al_context);
	BSTONE_ASSERT(al_symbols_);
	BSTONE_ASSERT(al_symbols_->alcMakeContextCurrent);
	BSTONE_ASSERT(al_symbols_->alcDestroyContext);
	const auto make_current_result = al_symbols_->alcMakeContextCurrent(nullptr);
#ifdef NDEBUG
	std::ignore = make_current_result;
#else
	BSTONE_ASSERT(make_current_result == ALC_TRUE);
#endif // NDEBUG
	al_symbols_->alcDestroyContext(al_context);
}

OalContextResource make_oal_context(const OalAlSymbols& al_symbols, ALCdevice& al_device, const ALCint* al_context_attributes)
try {
	if (!al_symbols.alcCreateContext)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"alcCreateContext\".");
	}

	const auto al_context = al_symbols.alcCreateContext(&al_device, al_context_attributes);

	if (!al_context)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create a context.");
	}

	return OalContextResource{al_context, OalContextDeleter{al_symbols}};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

OalBufferDeleter::OalBufferDeleter() noexcept = default;

OalBufferDeleter::OalBufferDeleter(const OalBufferDeleter& rhs) noexcept
	:
	al_symbols_{rhs.al_symbols_}
{
}

OalBufferDeleter::OalBufferDeleter(const OalAlSymbols& al_symbols) noexcept
	:
	al_symbols_{&al_symbols}
{
	BSTONE_ASSERT(al_symbols_->alDeleteBuffers);
}

void OalBufferDeleter::operator=(OalBufferDeleter&& rhs) noexcept
{
	al_symbols_ = rhs.al_symbols_;
}

void OalBufferDeleter::operator()(ALuint al_buffer) const noexcept
{
	BSTONE_ASSERT(al_buffer != AL_NONE);
	BSTONE_ASSERT(al_symbols_);
	BSTONE_ASSERT(al_symbols_->alDeleteBuffers);

	al_symbols_->alDeleteBuffers(1, &al_buffer);
}

OalBufferResource make_oal_buffer(const OalAlSymbols& al_symbols)
try {
	if (!al_symbols.alGenBuffers)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"alGenBuffers\".");
	}

	if (!al_symbols.alIsBuffer)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"alIsBuffer\".");
	}

	auto al_buffer = ALuint{};
	al_symbols.alGenBuffers(1, &al_buffer);

	const auto is_al_buffer = (al_symbols.alIsBuffer(al_buffer) == AL_TRUE);

	if (!is_al_buffer)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create a buffer.");
	}

	return OalBufferResource{al_buffer, OalBufferDeleter{al_symbols}};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

OalSourceDeleter::OalSourceDeleter() noexcept = default;

OalSourceDeleter::OalSourceDeleter(const OalSourceDeleter& rhs) noexcept
	:
	al_symbols_{rhs.al_symbols_}
{
}

OalSourceDeleter::OalSourceDeleter(const OalAlSymbols& al_symbols) noexcept
	:
	al_symbols_{&al_symbols}
{
	BSTONE_ASSERT(al_symbols_->alDeleteSources);
}

void OalSourceDeleter::operator=(OalSourceDeleter&& rhs) noexcept
{
	al_symbols_ = rhs.al_symbols_;
}

void OalSourceDeleter::operator()(ALuint al_source) const noexcept
{
	BSTONE_ASSERT(al_source != AL_NONE);
	BSTONE_ASSERT(al_symbols_);
	BSTONE_ASSERT(al_symbols_->alDeleteSources);

	al_symbols_->alDeleteSources(1, &al_source);
}

OalSourceResource make_oal_source(const OalAlSymbols& al_symbols)
try {
	if (!al_symbols.alGenSources)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"alGenSources\".");
	}

	if (!al_symbols.alIsSource)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"alIsSource\".");
	}

	auto al_source = ALuint{};
	al_symbols.alGenSources(1, &al_source);
	const auto is_al_source = (al_symbols.alIsSource(al_source) == AL_TRUE);

	if (!is_al_source)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create a source.");
	}

	return OalSourceResource{al_source, OalSourceDeleter{al_symbols}};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // bstone
