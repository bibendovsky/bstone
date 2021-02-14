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


#ifndef BSTONE_OAL_RESOURCE_INCLUDED
#define BSTONE_OAL_RESOURCE_INCLUDED


#include <type_traits>

#include "al.h"
#include "alc.h"

#include "bstone_oal_symbols.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename TResource,
	typename TDeleter
>
class OalResource
{
public:
	using Resource = TResource;
	using Deleter = TDeleter;


	static_assert(
		std::is_same<Resource, ALCdevice*>::value ||
			std::is_same<Resource, ALCcontext*>::value ||
			std::is_same<Resource, ALuint>::value,
		"Unsupported resource type."
	);


	OalResource() noexcept = default;

	OalResource(
		Resource resource,
		Deleter deleter) noexcept;

	OalResource(
		const OalResource& rhs) = delete;

	OalResource(
		OalResource&& rhs) noexcept;

	template<
		typename UResource = Resource,
		std::enable_if_t<std::is_pointer<UResource>::value, int> = 0
	>
	void operator=(
		std::nullptr_t) noexcept
	{
		reset();
	}

	OalResource& operator=(
		const OalResource& rhs) = delete;

	void operator=(
		OalResource&& rhs) noexcept;

	~OalResource();


	bool is_empty() const noexcept;

	TResource get() const noexcept;

	void reset() noexcept;

	void reset(
		Resource resource) noexcept;



private:
	Resource resource_{};
	Deleter deleter_{};
}; // OalResource

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

template<
	typename TResource,
	typename TDeleter
>
OalResource<TResource, TDeleter>::OalResource(
	Resource resource,
	Deleter deleter) noexcept
	:
	resource_{resource},
	deleter_{!is_empty() ? deleter : Deleter{}}
{
}

template<
	typename TResource,
	typename TDeleter
>
OalResource<TResource, TDeleter>::OalResource(
	OalResource&& rhs) noexcept
{
	std::swap(resource_, rhs.resource_);
	std::swap(deleter_, rhs.deleter_);
}

template<
	typename TResource,
	typename TDeleter
>
void OalResource<TResource, TDeleter>::operator=(
	OalResource&& rhs) noexcept
{
	std::swap(resource_, rhs.resource_);
	std::swap(deleter_, rhs.deleter_);
}

template<
	typename TResource,
	typename TDeleter
>
OalResource<TResource, TDeleter>::~OalResource()
{
	reset();
}

template<
	typename TResource,
	typename TDeleter
>
bool OalResource<TResource, TDeleter>::is_empty() const noexcept
{
	return resource_ == Resource{};
}

template<
	typename TResource,
	typename TDeleter
>
TResource OalResource<TResource, TDeleter>::get() const noexcept
{
	return resource_;
}

template<
	typename TResource,
	typename TDeleter
>
void OalResource<TResource, TDeleter>::reset() noexcept
{
	if (is_empty())
	{
		return;
	}

	deleter_(resource_);
	resource_ = Resource{};
}

template<
	typename TResource,
	typename TDeleter
>
void OalResource<TResource, TDeleter>::reset(
	Resource resource) noexcept
{
	reset();

	resource_ = resource;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalDeviceDeleter
{
public:
	OalDeviceDeleter() noexcept;

	OalDeviceDeleter(
		const OalDeviceDeleter& rhs);

	explicit OalDeviceDeleter(
		const OalSymbols* oal_symbols) noexcept;

	void operator=(
		OalDeviceDeleter&& rhs) noexcept;

	void operator()(
		ALCdevice* al_device) const noexcept;


private:
	const OalSymbols* oal_symbols_{};
}; // OalDeviceDeleter

using OalDeviceResource = OalResource<ALCdevice*, OalDeviceDeleter>;


OalDeviceResource make_oal_device(
	const OalSymbols* oal_symbols,
	const char* device_name);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalContextDeleter
{
public:
	OalContextDeleter() noexcept;

	OalContextDeleter(
		const OalContextDeleter& rhs) noexcept;

	explicit OalContextDeleter(
		const OalSymbols* oal_symbols) noexcept;

	void operator=(
		OalContextDeleter&& rhs) noexcept;

	void operator()(
		ALCcontext* al_context) const noexcept;


private:
	const OalSymbols* oal_symbols_{};
}; // OalContextDeleter

using OalContextResource = OalResource<ALCcontext*, OalContextDeleter>;


OalContextResource make_oal_context(
	const OalSymbols* oal_symbols,
	ALCdevice* al_device,
	const ALCint* al_context_attributes);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalBufferDeleter
{
public:
	OalBufferDeleter() noexcept;

	OalBufferDeleter(
		const OalBufferDeleter& rhs) noexcept;

	explicit OalBufferDeleter(
		const OalSymbols* oal_symbols) noexcept;

	void operator=(
		OalBufferDeleter&& rhs) noexcept;

	void operator()(
		ALuint al_buffer) const noexcept;


private:
	const OalSymbols* oal_symbols_{};
}; // OalBufferDeleter

using OalBufferResource = OalResource<ALuint, OalBufferDeleter>;


OalBufferResource make_oal_buffer(
	const OalSymbols* oal_symbols);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class OalSourceDeleter
{
public:
	OalSourceDeleter() noexcept;

	OalSourceDeleter(
		const OalSourceDeleter& rhs) noexcept;

	explicit OalSourceDeleter(
		const OalSymbols* oal_symbols) noexcept;

	void operator=(
		OalSourceDeleter&& rhs) noexcept;

	void operator()(
		ALuint al_source) const noexcept;


private:
	const OalSymbols* oal_symbols_{};
}; // OalSourceDeleter

using OalSourceResource = OalResource<ALuint, OalSourceDeleter>;


OalSourceResource make_oal_source(
	const OalSymbols* oal_symbols);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_OAL_RESOURCE_INCLUDED
