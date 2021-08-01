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


#include "bstone_oal_resource.h"


#include <cassert>

#include <utility>

#include "bstone_exception.h"
#include "bstone_not_null.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalResourceNullException :
	public Exception
{
public:
	explicit OalResourceNullException()
		:
		Exception{"OAL_RESOURCE", "Null value."}
	{
	}
}; // OalResourceNullException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalDeviceDeleter::OalDeviceDeleter() noexcept = default;

OalDeviceDeleter::OalDeviceDeleter(
	const OalDeviceDeleter& rhs)
	:
	oal_alc_symbols_{rhs.oal_alc_symbols_}
{
}

OalDeviceDeleter::OalDeviceDeleter(
	const OalAlcSymbols& oal_alc_symbols) noexcept
	:
	oal_alc_symbols_{&oal_alc_symbols}
{
	assert(oal_alc_symbols_->alcCloseDevice);
}

void OalDeviceDeleter::operator=(
	OalDeviceDeleter&& rhs) noexcept
{
	oal_alc_symbols_ = rhs.oal_alc_symbols_;
}

void OalDeviceDeleter::operator()(
	::ALCdevice* alc_device) const noexcept
{
	assert(alc_device);

	assert(oal_alc_symbols_);
	assert(oal_alc_symbols_->alcCloseDevice);

	const auto alc_result = oal_alc_symbols_->alcCloseDevice(alc_device);

#if NDEBUG
	static_cast<void>(alc_result);
#else
	assert(alc_result != ALC_FALSE);
#endif // NDEBUG
}


class OalDeviceResourceException :
	public Exception
{
public:
	explicit OalDeviceResourceException(
		const char* message)
		:
		Exception{"OAL_DEVICE_RESOURCE", message}
	{
	}
}; // OalDeviceResourceException

OalDeviceResource make_oal_device(
	const OalAlcSymbols& oal_alc_symbols,
	const char* device_name)
{
	not_null<OalResourceNullException>(oal_alc_symbols.alcOpenDevice);

	const auto alc_device = oal_alc_symbols.alcOpenDevice(device_name);

	if (!alc_device)
	{
		throw OalDeviceResourceException{"Failed to open a device."};
	}

	return OalDeviceResource{alc_device, OalDeviceDeleter{oal_alc_symbols}};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalContextDeleter::OalContextDeleter() noexcept = default;

OalContextDeleter::OalContextDeleter(
	const OalContextDeleter& rhs) noexcept
	:
	oal_alc_symbols_{rhs.oal_alc_symbols_}
{
}

OalContextDeleter::OalContextDeleter(
	const OalAlcSymbols& oal_alc_symbols) noexcept
	:
	oal_alc_symbols_{&oal_alc_symbols}
{
	assert(oal_alc_symbols_);
	assert(oal_alc_symbols_->alcGetCurrentContext);
	assert(oal_alc_symbols_->alcMakeContextCurrent);
	assert(oal_alc_symbols_->alcDestroyContext);
}

void OalContextDeleter::operator=(
	OalContextDeleter&& rhs) noexcept
{
	oal_alc_symbols_ = rhs.oal_alc_symbols_;
}

void OalContextDeleter::operator()(
	::ALCcontext* alc_context) const noexcept
{
	assert(alc_context);

	assert(oal_alc_symbols_);
	assert(oal_alc_symbols_->alcGetCurrentContext);
	assert(oal_alc_symbols_->alcMakeContextCurrent);
	assert(oal_alc_symbols_->alcDestroyContext);

	const auto alc_current_context = oal_alc_symbols_->alcGetCurrentContext();

	if (alc_current_context == alc_context)
	{
		const auto alc_result = oal_alc_symbols_->alcMakeContextCurrent(nullptr);

#if NDEBUG
		static_cast<void>(alc_result);
#else
		assert(alc_result != ALC_FALSE);
#endif // NDEBUG
	}

	oal_alc_symbols_->alcDestroyContext(alc_context);
}


class OalContextResourceException :
	public Exception
{
public:
	explicit OalContextResourceException(
		const char* message)
		:
		Exception{"OAL_CONTEXT_RESOURCE", message}
	{
	}
}; // OalContextResourceException

OalContextResource make_oal_context(
	const OalAlcSymbols& oal_alc_symbols,
	::ALCdevice* alc_device,
	const ::ALCint* al_context_attributes)
{
	not_null<OalResourceNullException>(oal_alc_symbols.alcCreateContext);
	not_null<OalResourceNullException>(alc_device);

	const auto alc_context = oal_alc_symbols.alcCreateContext(alc_device, al_context_attributes);

	if (!alc_context)
	{
		throw OalContextResourceException{"Failed to create a context."};
	}

	return OalContextResource{alc_context, OalContextDeleter{oal_alc_symbols}};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalBufferDeleter::OalBufferDeleter() noexcept = default;

OalBufferDeleter::OalBufferDeleter(
	const OalBufferDeleter& rhs) noexcept
	:
	oal_al_symbols_{rhs.oal_al_symbols_}
{
}

OalBufferDeleter::OalBufferDeleter(
	const OalAlSymbols& oal_al_symbols) noexcept
	:
	oal_al_symbols_{&oal_al_symbols}
{
	assert(oal_al_symbols_->alDeleteBuffers);
}

void OalBufferDeleter::operator=(
	OalBufferDeleter&& rhs) noexcept
{
	oal_al_symbols_ = rhs.oal_al_symbols_;
}

void OalBufferDeleter::operator()(
	::ALuint al_buffer) const noexcept
{
	assert(al_buffer != AL_NONE);

	assert(oal_al_symbols_);
	assert(oal_al_symbols_->alDeleteBuffers);

	oal_al_symbols_->alDeleteBuffers(1, &al_buffer);
}


OalBufferResource make_oal_buffer(
	const OalAlSymbols& oal_al_symbols)
{
	not_null<OalResourceNullException>(oal_al_symbols.alGenBuffers);
	not_null<OalResourceNullException>(oal_al_symbols.alIsBuffer);

	auto al_buffer = ::ALuint{};
	oal_al_symbols.alGenBuffers(1, &al_buffer);

	const auto is_al_buffer = (oal_al_symbols.alIsBuffer(al_buffer) != AL_FALSE);

	if (!is_al_buffer)
	{
		throw OalContextResourceException{"Failed to create a buffer."};
	}

	return OalBufferResource{al_buffer, OalBufferDeleter{oal_al_symbols}};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalSourceDeleter::OalSourceDeleter() noexcept = default;

OalSourceDeleter::OalSourceDeleter(
	const OalSourceDeleter& rhs) noexcept
	:
	oal_al_symbols_{rhs.oal_al_symbols_}
{
}

OalSourceDeleter::OalSourceDeleter(
	const OalAlSymbols& oal_al_symbols) noexcept
	:
	oal_al_symbols_{&oal_al_symbols}
{
	assert(oal_al_symbols_->alDeleteSources);
}

void OalSourceDeleter::operator=(
	OalSourceDeleter&& rhs) noexcept
{
	oal_al_symbols_ = rhs.oal_al_symbols_;
}

void OalSourceDeleter::operator()(
	::ALuint al_source) const noexcept
{
	assert(al_source != AL_NONE);

	assert(oal_al_symbols_);
	assert(oal_al_symbols_->alDeleteSources);

	oal_al_symbols_->alDeleteSources(1, &al_source);
}


OalSourceResource make_oal_source(
	const OalAlSymbols& oal_al_symbols)
{
	not_null<OalResourceNullException>(oal_al_symbols.alGenSources);
	not_null<OalResourceNullException>(oal_al_symbols.alIsSource);

	auto al_source = ::ALuint{};
	oal_al_symbols.alGenSources(1, &al_source);

	const auto is_al_source = (oal_al_symbols.alIsSource(al_source) != AL_FALSE);

	if (!is_al_source)
	{
		throw OalContextResourceException{"Failed to create a source."};
	}

	return OalSourceResource{al_source, OalSourceDeleter{oal_al_symbols}};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
