/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Universal RAII wrapper for resource handles.

/*
Notes:
  - Resource, deleter and empty value must not throw an exception.
*/

#ifndef BSTONE_UNIQUE_RESOURCE_INCLUDED
#define BSTONE_UNIQUE_RESOURCE_INCLUDED

#include <type_traits>

#include "bstone_assert.h"
#include "bstone_utility.h"

namespace bstone {

namespace detail {

template<typename TResource>
struct UniqueResourceIsPointer : std::integral_constant<bool, std::is_pointer<TResource>::value> {};

template<typename TResource>
struct UniqueResourceIsNonPointer : std::integral_constant<bool, std::is_integral<TResource>::value> {};

template<typename TResource>
struct UniqueResourceIsTypeValid :
	std::integral_constant<bool, UniqueResourceIsPointer<TResource>::value ||
		UniqueResourceIsNonPointer<TResource>::value>
{};

// ==========================================================================

template<typename TResource, typename TDeleter, bool TIsMonostate>
struct UniqueResourceStorage;

// Unique state.
template<typename TResource, typename TDeleter>
struct UniqueResourceStorage<TResource, TDeleter, false>
{
public:
	using Resource = TResource;
	using Deleter = TDeleter;

public:
	Resource resource_{};
	Deleter deleter_{};

public:
	UniqueResourceStorage(Resource resource, Deleter deleter) noexcept
		:
		resource_{resource},
		deleter_{deleter}
	{}

	const Deleter& get_deleter() const noexcept
	{
		return deleter_;
	}

	void destruct() noexcept
	{
		deleter_(resource_);
	}

	void swap(UniqueResourceStorage& rhs) noexcept
	{
		swop(resource_, rhs.resource_);
		swop(deleter_, rhs.deleter_);
	}
};

// Monostate.
template<typename TResource, typename TDeleter>
struct UniqueResourceStorage<TResource, TDeleter, true>
{
public:
	using Resource = TResource;
	using Deleter = TDeleter;

public:
	Resource resource_{};

public:
	UniqueResourceStorage(Resource resource, Deleter) noexcept
		:
		resource_{resource}
	{}

	void destruct() noexcept
	{
		Deleter{}(resource_);
	}

	void swap(UniqueResourceStorage& rhs) noexcept
	{
		bstone::swop(resource_, rhs.resource_);
	}
};

} // namespace detail

// ==========================================================================

template<typename TResource>
struct UniqueResourceDefaultEmptyValue
{
public:
	static_assert(detail::UniqueResourceIsTypeValid<TResource>::value, "Unsupported resource type.");

public:
	using Resource = TResource;

public:
	Resource operator()() const noexcept
	{
		return Resource{};
	}
};

// ==========================================================================

template<
	typename TResource,
	typename TDeleter,
	typename TEmptyValue = UniqueResourceDefaultEmptyValue<TResource>>
class UniqueResource
{
public:
	static_assert(detail::UniqueResourceIsTypeValid<TResource>::value, "Unsupported resource type.");
	static_assert(std::is_class<TDeleter>::value, "Unsupported deleter type.");
	static_assert(std::is_empty<TEmptyValue>::value, "Unsupported empty-value type.");

public:
	using Resource = TResource;
	using Deleter = TDeleter;

public:
	static constexpr TResource get_empty_value() noexcept
	{
		return TEmptyValue{}();
	}

	template<
		typename UDeleter = Deleter,
		std::enable_if_t<std::is_empty<UDeleter>::value, int> = 0>
	UniqueResource() noexcept
		:
		storage_{get_empty_value(), Deleter{}}
	{}

	template<
		typename UDeleter = Deleter,
		std::enable_if_t<!std::is_empty<UDeleter>::value, int> = 0>
	explicit UniqueResource(Deleter deleter) noexcept
		:
		storage_{get_empty_value(), deleter}
	{}

	template<
		typename UDeleter = Deleter,
		std::enable_if_t<std::is_empty<UDeleter>::value, int> = 0>
	explicit UniqueResource(Resource resource) noexcept
		:
		storage_{resource, Deleter{}}
	{}

	template<
		typename UDeleter = Deleter,
		std::enable_if_t<!std::is_empty<UDeleter>::value, int> = 0>
	UniqueResource(Resource resource, Deleter deleter) noexcept
		:
		storage_{resource, deleter}
	{}

	template<
		typename UDeleter = Deleter,
		std::enable_if_t<std::is_empty<UDeleter>::value, int> = 0>
	UniqueResource(UniqueResource&& rhs) noexcept
		:
		storage_{get_empty_value(), Deleter{}}
	{
		storage_.swap(rhs.storage_);
	}

	template<
		typename UDeleter = Deleter,
		std::enable_if_t<!std::is_empty<UDeleter>::value, int> = 0>
	UniqueResource(UniqueResource&& rhs) noexcept
		:
		storage_{get_empty_value(), rhs.storage_.get_deleter()}
	{
		storage_.swap(rhs.storage_);
	}

	UniqueResource& operator=(UniqueResource&& rhs) noexcept
	{
		reset();
		storage_.swap(rhs.storage_);
		return *this;
	}

	~UniqueResource()
	{
		reset();
	}

	const Resource& get() const noexcept
	{
		return storage_.resource_;
	}

	bool is_empty() const noexcept
	{
		return get() == get_empty_value();
	}

	void reset() noexcept
	{
		if (is_empty())
		{
			return;
		}

		storage_.destruct();
		storage_.resource_ = get_empty_value();
	}

	void reset(const Resource resource) noexcept
	{
		reset();
		storage_.resource_ = resource;
	}

	Resource release() noexcept
	{
		auto result = get_empty_value();
		bstone::swop(result, storage_.resource_);
		return result;
	}

	void swap(UniqueResource& rhs) noexcept
	{
		storage_.swap(rhs.storage_);
	}

	template<
		typename UniqueResource = Resource,
		std::enable_if_t<detail::UniqueResourceIsPointer<UniqueResource>::value, int> = 0>
	typename std::add_lvalue_reference_t<std::remove_pointer_t<UniqueResource>> operator*() const noexcept
	{
		BSTONE_ASSERT(get() != nullptr);
		return *get();
	}

	template<
		typename UniqueResource = Resource,
		std::enable_if_t<detail::UniqueResourceIsPointer<UniqueResource>::value, int> = 0>
	Resource operator->() const noexcept
	{
		BSTONE_ASSERT(get() != nullptr);
		return get();
	}

private:
	using Storage = detail::UniqueResourceStorage<Resource, Deleter, std::is_empty<Deleter>::value>;

public:
	Storage storage_;
};

// ==========================================================================

template<
	typename TResource,
	typename TDeleter,
	typename TEmptyValue>
inline bool operator==(
	const UniqueResource<TResource, TDeleter, TEmptyValue>& lhs,
	const UniqueResource<TResource, TDeleter, TEmptyValue>& rhs) noexcept
{
	return lhs.get() == rhs.get();
}

template<
	typename TResource,
	typename TDeleter,
	typename TEmptyValue>
inline bool operator!=(
	const UniqueResource<TResource, TDeleter, TEmptyValue>& lhs,
	const UniqueResource<TResource, TDeleter, TEmptyValue>& rhs) noexcept
{
	return !(lhs == rhs);
}

// ==========================================================================

template<
	typename TResource,
	typename TDeleter,
	typename TEmptyValue>
inline bool operator==(
	const UniqueResource<TResource, TDeleter, TEmptyValue>& lhs,
	std::nullptr_t) noexcept
{
	return lhs.get() == nullptr;
}

template<
	typename TResource,
	typename TDeleter,
	typename TEmptyValue>
inline bool operator!=(
	const UniqueResource<TResource, TDeleter, TEmptyValue>& lhs,
	std::nullptr_t) noexcept
{
	return !(lhs == nullptr);
}

template<
	typename TResource,
	typename TDeleter,
	typename TEmptyValue>
inline bool operator==(
	std::nullptr_t,
	const UniqueResource<TResource, TDeleter, TEmptyValue>& rhs) noexcept
{
	return rhs == nullptr;
}

template<
	typename TResource,
	typename TDeleter,
	typename TEmptyValue>
inline bool operator!=(
	std::nullptr_t,
	const UniqueResource<TResource, TDeleter, TEmptyValue>& rhs) noexcept
{
	return !(rhs == nullptr);
}

} // namespace bstone

#endif // BSTONE_UNIQUE_RESOURCE_INCLUDED
