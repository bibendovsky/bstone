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
// Generic list for std::unique_ptr.
//


#ifndef BSTONE_UPTR_RESOURCE_LIST_INCLUDED
#define BSTONE_UPTR_RESOURCE_LIST_INCLUDED


#include <list>
#include <memory>
#include <type_traits>


namespace bstone
{


template<
	typename TResource,
	typename TFactory,
	typename TException
>
class UPtrResourceList
{
	static_assert(std::is_class<TResource>::value, "Expected class type.");
	static_assert(std::is_class<TFactory>::value, "Expected class type.");


public:
	using Resource = TResource;
	using ResourcePtr = TResource*;


	UPtrResourceList()
		:
		list_{}
	{
	}

	UPtrResourceList(
		const UPtrResourceList& rhs) = delete;

	UPtrResourceList(
		UPtrResourceList&& rhs) = default;


	template<typename... TArgs>
	ResourcePtr add(
		TArgs&&... args)
	{
		auto resource = TFactory::create(std::forward<TArgs>(args)...);

		list_.emplace_back(std::move(resource));

		return list_.back().get();
	}

	template<typename UResource>
	void remove(
		const UResource& resource)
	{
		if (!resource)
		{
			throw TException{"Null resource."};
		}

		list_.remove_if(
			[&](const auto& item)
			{
				return item.get() == resource;
			}
		);
	}


private:
	using ResourceUPtr = std::unique_ptr<Resource>;
	using List = std::list<ResourceUPtr>;

	List list_;
}; // UPtrResourceList


} // bstone


#endif // BSTONE_UPTR_RESOURCE_LIST_INCLUDED
