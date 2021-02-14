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


//
// RAII wrapper for system resources.
//


#ifndef BSTONE_UNIQUE_RESOURCE_INCLUDED
#define BSTONE_UNIQUE_RESOURCE_INCLUDED


#include <algorithm>
#include <type_traits>


#define BSTONE_T_ENABLE_IF(...) typename std::enable_if<(__VA_ARGS__), int>::type = 0


namespace bstone
{


template<
	typename TResource,
	void (*TDeleter)(const TResource& resource) noexcept>
class UniqueResource
{
public:
	using Resource = TResource;


	UniqueResource() noexcept
		:
		resource_{}
	{
	}

	explicit UniqueResource(
		const Resource resource) noexcept
		:
		resource_{resource}
	{
	}

	UniqueResource(
		const UniqueResource& rhs) = delete;

	UniqueResource(
		UniqueResource&& rhs) noexcept
		:
		resource_{}
	{
		std::swap(resource_, rhs.resource_);
	}

	UniqueResource& operator=(
		const UniqueResource& rhs) = delete;

	UniqueResource& operator=(
		UniqueResource&& rhs) noexcept
	{
		std::swap(resource_, rhs.resource_);

		return *this;
	}

	~UniqueResource()
	{
		close();
	}


	bool is_valid() const noexcept
	{
		return resource_ != Resource{};
	}

	bool is_invalid() const noexcept
	{
		return resource_ == Resource{};
	}

	const Resource& get() const noexcept
	{
		return resource_;
	}

	void reset()
	{
		close();
	}

	void reset(
		const Resource& resource)
	{
		close();

		resource_ = resource;
	}


	explicit operator bool() const noexcept
	{
		return is_valid();
	}

	template<
		typename TResourceProxy = Resource,
		BSTONE_T_ENABLE_IF(std::is_pointer<TResourceProxy>::value)
	>
	typename std::add_lvalue_reference_t<std::remove_pointer_t<TResourceProxy>> operator*() const noexcept
	{
		return *get();
	}

	template<
		typename TResourceProxy = Resource,
		BSTONE_T_ENABLE_IF(
			std::is_pointer<TResourceProxy>::value &&
			(std::is_class<std::remove_pointer_t<TResourceProxy>>::value ||
				std::is_union<std::remove_pointer_t<TResourceProxy>>::value)
		)
	>
	Resource operator->() const noexcept
	{
		return resource_;
	}

private:
	Resource resource_;


	void close() noexcept
	{
		if (is_invalid())
		{
			return;
		}

		TDeleter(resource_);
		resource_ = {};
	}
}; // Handle


} // UniqueResource


#endif // !BSTONE_UNIQUE_RESOURCE_INCLUDED
