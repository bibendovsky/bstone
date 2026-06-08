/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_OAL_RESOURCE_INCLUDED
#define BSTONE_OAL_RESOURCE_INCLUDED

#include "al.h"
#include "alc.h"
#include "bstone_assert.h"
#include "bstone_oal_symbols.h"
#include <type_traits>
#include <utility>

namespace bstone {

template<typename TResource, typename TDeleter>
class OalResource
{
public:
	using Resource = TResource;
	using Deleter = TDeleter;

	static_assert(
		std::is_same<Resource, ALCdevice*>::value ||
			std::is_same<Resource, ALCcontext*>::value ||
			std::is_same<Resource, ALuint>::value,
		"Unsupported resource type.");

	OalResource() = default;

	OalResource(Resource resource, Deleter deleter)
		:
		resource_{resource},
		deleter_{deleter}
	{}

	OalResource(const OalResource& rhs) = delete;

	OalResource(OalResource&& rhs) noexcept
	{
		std::swap(resource_, rhs.resource_);
		std::swap(deleter_, rhs.deleter_);
	}

	template<
		typename UResource = Resource,
		std::enable_if_t<std::is_pointer<UResource>::value, int> = 0
	>
	void operator=(std::nullptr_t)
	{
		reset();
	}

	OalResource& operator=(const OalResource& rhs) = delete;

	void operator=(OalResource&& rhs) noexcept
	{
		std::swap(resource_, rhs.resource_);
		std::swap(deleter_, rhs.deleter_);
	}

	~OalResource()
	{
		reset();
	}

	bool is_empty() const
	{
		return resource_ == Resource{};
	}

	TResource get() const
	{
		return resource_;
	}

	void reset()
	{
		if (is_empty())
			return;
		deleter_(resource_);
		resource_ = Resource{};
	}

	void reset(Resource resource)
	{
		reset();
		resource_ = resource;
	}

	template<
		typename UResource = Resource,
		std::enable_if_t<std::is_pointer<UResource>::value, int> = 0
	>
	explicit operator bool() const
	{
		return !is_empty();
	}

	template<
		typename UResource = Resource,
		std::enable_if_t<std::is_pointer<UResource>::value, int> = 0
	>
	auto& operator*()
	{
		BSTONE_ASSERT(!is_empty());
		return *resource_;
	}

private:
	Resource resource_{};
	Deleter deleter_{};
};

// =====================================

class OalDeviceDeleter
{
public:
	OalDeviceDeleter() = default;
	OalDeviceDeleter(const OalDeviceDeleter& rhs);
	explicit OalDeviceDeleter(const OalAlSymbols& al_symbols);
	void operator=(OalDeviceDeleter&& rhs) noexcept;
	void operator()(ALCdevice* alc_device) const;

private:
	const OalAlSymbols* al_symbols_{};
};

using OalDeviceResource = OalResource<ALCdevice*, OalDeviceDeleter>;

OalDeviceResource make_oal_device(const OalAlSymbols& al_symbols, const char* device_name);

// =====================================

class OalContextDeleter
{
public:
	OalContextDeleter() = default;
	OalContextDeleter(const OalContextDeleter& rhs);
	explicit OalContextDeleter(const OalAlSymbols& al_symbols);
	void operator=(OalContextDeleter&& rhs) noexcept;
	void operator()(ALCcontext* alc_context) const;

private:
	const OalAlSymbols* al_symbols_{};
};

using OalContextResource = OalResource<ALCcontext*, OalContextDeleter>;

OalContextResource make_oal_context(const OalAlSymbols& al_symbols, ALCdevice& al_device, const ALCint* al_context_attributes);

// =====================================

class OalBufferDeleter
{
public:
	OalBufferDeleter() = default;
	OalBufferDeleter(const OalBufferDeleter& rhs);
	explicit OalBufferDeleter(const OalAlSymbols& al_symbols);
	void operator=(OalBufferDeleter&& rhs) noexcept;
	void operator()(ALuint al_buffer) const;

private:
	const OalAlSymbols* al_symbols_{};
};

using OalBufferResource = OalResource<ALuint, OalBufferDeleter>;

OalBufferResource make_oal_buffer(const OalAlSymbols& oal_al_symbols);

// =====================================

class OalSourceDeleter
{
public:
	OalSourceDeleter() = default;
	OalSourceDeleter(const OalSourceDeleter& rhs);
	explicit OalSourceDeleter(const OalAlSymbols& al_symbols);
	void operator=(OalSourceDeleter&& rhs) noexcept;
	void operator()(ALuint al_source) const;

private:
	const OalAlSymbols* al_symbols_{};
};

using OalSourceResource = OalResource<ALuint, OalSourceDeleter>;

OalSourceResource make_oal_source(const OalAlSymbols& al_symbols);

} // namespace bstone

#endif // BSTONE_OAL_RESOURCE_INCLUDED
