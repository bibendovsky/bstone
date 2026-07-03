/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_OAL_RESOURCE_INCLUDED
#define BSTONE_OAL_RESOURCE_INCLUDED

#include "al.h"
#include "alc.h"
#include "bstone_assert.h"
#include <cstddef>
#include <memory>
#include <type_traits>

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

	inline static constexpr Resource empty_value{};

	OalResource() = default;

	explicit OalResource(Resource resource)
		:
		resource_{resource}
	{}

	OalResource(const OalResource& rhs) = delete;

	OalResource(OalResource&& rhs) noexcept
		:
		resource_{rhs.resource_}
	{
		rhs.resource_ = empty_value;
	}

	template<typename UResource = Resource>
	requires(std::is_pointer_v<UResource>)
	void operator=(const std::nullptr_t&)
	{
		reset();
	}

	OalResource& operator=(const OalResource& rhs) = delete;

	void operator=(OalResource&& rhs) noexcept
	{
		BSTONE_ASSERT(std::addressof(rhs) != this);
		delete_resource();
		resource_ = rhs.resource_;
		rhs.resource_ = empty_value;
	}

	~OalResource()
	{
		reset();
	}

	bool is_empty() const
	{
		return resource_ == empty_value;
	}

	TResource get() const
	{
		return resource_;
	}

	void reset()
	{
		reset(empty_value);
	}

	void reset(Resource resource)
	{
		delete_resource();
		resource_ = resource;
	}

	template<typename UResource = Resource>
	requires(std::is_pointer_v<UResource>)
	explicit operator bool() const
	{
		return !is_empty();
	}

	template<typename UResource = Resource>
	requires(std::is_pointer_v<UResource>)
	auto& operator*()
	{
		BSTONE_ASSERT(!is_empty());
		return *resource_;
	}

private:
	Resource resource_{};

	void delete_resource() const
	{
		Deleter{}(resource_);
	}
};

// =====================================

struct OalDeviceDeleter
{
	void operator()(ALCdevice* alc_device) const;
};

using OalDeviceResource = OalResource<ALCdevice*, OalDeviceDeleter>;

OalDeviceResource make_oal_device(const char* device_name);

// =====================================

struct OalContextDeleter
{
	void operator()(ALCcontext* alc_context) const;
};

using OalContextResource = OalResource<ALCcontext*, OalContextDeleter>;

OalContextResource make_oal_context(ALCdevice& al_device, const ALCint* al_context_attributes);

// =====================================

struct OalBufferDeleter
{
	void operator()(ALuint al_buffer) const;
};

using OalBufferResource = OalResource<ALuint, OalBufferDeleter>;

OalBufferResource make_oal_buffer();

// =====================================

struct OalSourceDeleter
{
	void operator()(ALuint al_source) const;
};

using OalSourceResource = OalResource<ALuint, OalSourceDeleter>;

OalSourceResource make_oal_source();

} // namespace bstone

#endif // BSTONE_OAL_RESOURCE_INCLUDED
