/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_OAL_RESOURCE_INCLUDED
#define BSTONE_OAL_RESOURCE_INCLUDED

#include <type_traits>
#include "al.h"
#include "alc.h"
#include "bstone_assert.h"
#include "bstone_oal_symbols.h"
#include "bstone_utility.h"

namespace bstone
{

template<typename TResource, typename TDeleter>
class OalResource
{
public:
	using Resource = TResource;
	using Deleter = TDeleter;

	static_assert(std::is_same<Resource, ALCdevice*>::value || std::is_same<Resource, ALCcontext*>::value || std::is_same<Resource, ALuint>::value, "Unsupported resource type.");

	OalResource() noexcept = default;

	OalResource(Resource resource, Deleter deleter) noexcept
		:
		resource_{resource},
		deleter_{deleter}
	{
	}

	OalResource(const OalResource& rhs) = delete;

	OalResource(OalResource&& rhs) noexcept
	{
		bstone::swop(resource_, rhs.resource_);
		bstone::swop(deleter_, rhs.deleter_);
	}

	template<
		typename UResource = Resource,
		std::enable_if_t<std::is_pointer<UResource>::value, int> = 0
	>
	void operator=(std::nullptr_t) noexcept
	{
		reset();
	}

	OalResource& operator=(const OalResource& rhs) = delete;

	void operator=(OalResource&& rhs) noexcept
	{
		bstone::swop(resource_, rhs.resource_);
		bstone::swop(deleter_, rhs.deleter_);
	}

	~OalResource()
	{
		reset();
	}

	bool is_empty() const noexcept
	{
		return resource_ == Resource{};
	}

	TResource get() const noexcept
	{
		return resource_;
	}

	void reset() noexcept
	{
		if (is_empty())
		{
			return;
		}

		deleter_(resource_);
		resource_ = Resource{};
	}

	void reset(Resource resource) noexcept
	{
		reset();
		resource_ = resource;
	}

	template<
		typename UResource = Resource,
		std::enable_if_t<std::is_pointer<UResource>::value, int> = 0
	>
	explicit operator bool() const noexcept
	{
		return !is_empty();
	}

	template<
		typename UResource = Resource,
		std::enable_if_t<std::is_pointer<UResource>::value, int> = 0
	>
	auto& operator*() noexcept
	{
		BSTONE_ASSERT(!is_empty());
		return *resource_;
	}

private:
	Resource resource_{};
	Deleter deleter_{};
}; // OalResource

// ==========================================================================

class OalDeviceDeleter
{
public:
	OalDeviceDeleter() noexcept;
	OalDeviceDeleter(const OalDeviceDeleter& rhs);
	explicit OalDeviceDeleter(const OalAlSymbols& al_symbols) noexcept;
	void operator=(OalDeviceDeleter&& rhs) noexcept;
	void operator()(ALCdevice* alc_device) const noexcept;

private:
	const OalAlSymbols* al_symbols_{};
}; // OalDeviceDeleter

using OalDeviceResource = OalResource<ALCdevice*, OalDeviceDeleter>;

OalDeviceResource make_oal_device(const OalAlSymbols& al_symbols, const char* device_name);

// ==========================================================================

class OalContextDeleter
{
public:
	OalContextDeleter() noexcept;
	OalContextDeleter(const OalContextDeleter& rhs) noexcept;
	explicit OalContextDeleter(const OalAlSymbols& al_symbols) noexcept;
	void operator=(OalContextDeleter&& rhs) noexcept;
	void operator()(ALCcontext* alc_context) const noexcept;

private:
	const OalAlSymbols* al_symbols_{};
}; // OalContextDeleter

using OalContextResource = OalResource<ALCcontext*, OalContextDeleter>;

OalContextResource make_oal_context(const OalAlSymbols& al_symbols, ALCdevice& al_device, const ALCint* al_context_attributes);

// ==========================================================================

class OalBufferDeleter
{
public:
	OalBufferDeleter() noexcept;
	OalBufferDeleter(const OalBufferDeleter& rhs) noexcept;
	explicit OalBufferDeleter(const OalAlSymbols& al_symbols) noexcept;
	void operator=(OalBufferDeleter&& rhs) noexcept;
	void operator()(ALuint al_buffer) const noexcept;

private:
	const OalAlSymbols* al_symbols_{};
}; // OalBufferDeleter

using OalBufferResource = OalResource<ALuint, OalBufferDeleter>;

OalBufferResource make_oal_buffer(const OalAlSymbols& oal_al_symbols);

// ==========================================================================

class OalSourceDeleter
{
public:
	OalSourceDeleter() noexcept;
	OalSourceDeleter(const OalSourceDeleter& rhs) noexcept;
	explicit OalSourceDeleter(const OalAlSymbols& al_symbols) noexcept;
	void operator=(OalSourceDeleter&& rhs) noexcept;
	void operator()(ALuint al_source) const noexcept;

private:
	const OalAlSymbols* al_symbols_{};
}; // OalSourceDeleter

using OalSourceResource = OalResource<ALuint, OalSourceDeleter>;

OalSourceResource make_oal_source(const OalAlSymbols& al_symbols);

} // bstone

#endif // !BSTONE_OAL_RESOURCE_INCLUDED
