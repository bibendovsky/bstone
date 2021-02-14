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
		Exception{"[OAL_RESOURCE] Null value."}
	{
	}
}; // OalResourceNullException

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalDeviceDeleter::OalDeviceDeleter() noexcept = default;

OalDeviceDeleter::OalDeviceDeleter(
	const OalDeviceDeleter& rhs)
	:
	oal_symbols_{rhs.oal_symbols_}
{
}

OalDeviceDeleter::OalDeviceDeleter(
	const OalSymbols* oal_symbols) noexcept
	:
	oal_symbols_{oal_symbols}
{
	assert(oal_symbols_ != nullptr);
	assert(oal_symbols_->alcCloseDevice != nullptr);
}

void OalDeviceDeleter::operator=(
	OalDeviceDeleter&& rhs) noexcept
{
	oal_symbols_ = rhs.oal_symbols_;
}

void OalDeviceDeleter::operator()(
	ALCdevice* al_device) const noexcept
{
	assert(al_device != nullptr);

	assert(oal_symbols_ != nullptr);
	assert(oal_symbols_->alcCloseDevice != nullptr);

	const auto al_result = oal_symbols_->alcCloseDevice(al_device);
	assert(al_result == ALC_TRUE);
}


class OalDeviceResourceException :
	public Exception
{
public:
	explicit OalDeviceResourceException(
		const char* message)
		:
		Exception{std::string{"[OAL_DEVICE_RESOURCE] "} + message}
	{
	}
}; // OalDeviceResourceException

OalDeviceResource make_oal_device(
	const OalSymbols* oal_symbols,
	const char* device_name)
{
	not_null<OalResourceNullException>(oal_symbols);
	not_null<OalResourceNullException>(oal_symbols->alcOpenDevice);

	const auto al_device = oal_symbols->alcOpenDevice(device_name);

	if (al_device != nullptr)
	{
		return OalDeviceResource{al_device, OalDeviceDeleter{oal_symbols}};
	}

	throw OalDeviceResourceException{"Failed to open a device."};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalContextDeleter::OalContextDeleter() noexcept = default;

OalContextDeleter::OalContextDeleter(
	const OalContextDeleter& rhs) noexcept
	:
	oal_symbols_{rhs.oal_symbols_}
{
}

OalContextDeleter::OalContextDeleter(
	const OalSymbols* oal_symbols) noexcept
	:
	oal_symbols_{oal_symbols}
{
	assert(oal_symbols_ != nullptr);
	assert(oal_symbols_->alcGetCurrentContext != nullptr);
	assert(oal_symbols_->alcMakeContextCurrent != nullptr);
	assert(oal_symbols_->alcDestroyContext != nullptr);
}

void OalContextDeleter::operator=(
	OalContextDeleter&& rhs) noexcept
{
	oal_symbols_ = rhs.oal_symbols_;
}

void OalContextDeleter::operator()(
	ALCcontext* al_context) const noexcept
{
	assert(al_context != nullptr);

	assert(oal_symbols_ != nullptr);
	assert(oal_symbols_->alcGetCurrentContext != nullptr);
	assert(oal_symbols_->alcMakeContextCurrent != nullptr);
	assert(oal_symbols_->alcDestroyContext != nullptr);

	const auto al_current_context = oal_symbols_->alcGetCurrentContext();

	if (al_current_context == al_context)
	{
		const auto al_result = oal_symbols_->alcMakeContextCurrent(nullptr);
		assert(al_result == ALC_TRUE);
	}

	oal_symbols_->alcDestroyContext(al_context);
}


class OalContextResourceException :
	public Exception
{
public:
	explicit OalContextResourceException(
		const char* message)
		:
		Exception{std::string{"[OAL_CONTEXT_RESOURCE] "} + message}
	{
	}
}; // OalContextResourceException

OalContextResource make_oal_context(
	const OalSymbols* oal_symbols,
	ALCdevice* al_device,
	const ALCint* al_context_attributes)
{
	not_null<OalResourceNullException>(oal_symbols);
	not_null<OalResourceNullException>(oal_symbols->alcCreateContext);
	not_null<OalResourceNullException>(al_device);

	const auto al_context = oal_symbols->alcCreateContext(al_device, al_context_attributes);

	if (al_context != nullptr)
	{
		return OalContextResource{al_context, OalContextDeleter{oal_symbols}};
	}

	throw OalContextResourceException{"Failed to create a context."};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalBufferDeleter::OalBufferDeleter() noexcept = default;

OalBufferDeleter::OalBufferDeleter(
	const OalBufferDeleter& rhs) noexcept
	:
	oal_symbols_{rhs.oal_symbols_}
{
}

OalBufferDeleter::OalBufferDeleter(
	const OalSymbols* oal_symbols) noexcept
	:
	oal_symbols_{oal_symbols}
{
	assert(oal_symbols_ != nullptr);
	assert(oal_symbols_->alDeleteBuffers != nullptr);
}

void OalBufferDeleter::operator=(
	OalBufferDeleter&& rhs) noexcept
{
	oal_symbols_ = rhs.oal_symbols_;
}

void OalBufferDeleter::operator()(
	ALuint al_buffer) const noexcept
{
	assert(al_buffer != 0);

	assert(oal_symbols_ != nullptr);
	assert(oal_symbols_->alDeleteBuffers != nullptr);

	oal_symbols_->alDeleteBuffers(1, &al_buffer);
}


OalBufferResource make_oal_buffer(
	const OalSymbols* oal_symbols)
{
	not_null<OalResourceNullException>(oal_symbols);
	not_null<OalResourceNullException>(oal_symbols->alGenBuffers);

	auto al_buffer = ALuint{};
	oal_symbols->alGenBuffers(1, &al_buffer);

	if (al_buffer != 0)
	{
		return OalBufferResource{al_buffer, OalBufferDeleter{oal_symbols}};
	}

	throw OalContextResourceException{"Failed to create a buffer."};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

OalSourceDeleter::OalSourceDeleter() noexcept = default;

OalSourceDeleter::OalSourceDeleter(
	const OalSourceDeleter& rhs) noexcept
	:
	oal_symbols_{rhs.oal_symbols_}
{
}

OalSourceDeleter::OalSourceDeleter(
	const OalSymbols* oal_symbols) noexcept
	:
	oal_symbols_{oal_symbols}
{
	assert(oal_symbols_ != nullptr);
	assert(oal_symbols_->alDeleteSources != nullptr);
}

void OalSourceDeleter::operator=(
	OalSourceDeleter&& rhs) noexcept
{
	oal_symbols_ = rhs.oal_symbols_;
}

void OalSourceDeleter::operator()(
	ALuint al_source) const noexcept
{
	assert(al_source != 0);

	assert(oal_symbols_ != nullptr);
	assert(oal_symbols_->alDeleteSources != nullptr);

	oal_symbols_->alDeleteSources(1, &al_source);
}


OalSourceResource make_oal_source(
	const OalSymbols* oal_symbols)
{
	not_null<OalResourceNullException>(oal_symbols);
	not_null<OalResourceNullException>(oal_symbols->alGenSources);

	auto al_source = ALuint{};
	oal_symbols->alGenSources(1, &al_source);

	if (al_source != 0)
	{
		return OalSourceResource{al_source, OalSourceDeleter{oal_symbols}};
	}

	throw OalContextResourceException{"Failed to create a source."};
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
