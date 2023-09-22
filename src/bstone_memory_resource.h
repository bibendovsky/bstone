/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_MEMORY_RESOURCE_INCLUDED)
#define BSTONE_MEMORY_RESOURCE_INCLUDED

#include <memory>

#include "bstone_int.h"
#include "bstone_memory.h"

namespace bstone {

class MemoryResource
{
public:
	MemoryResource() = default;
	virtual ~MemoryResource() = default;

	void* allocate(IntP size);
	void deallocate(void* ptr);

private:
	virtual void* do_allocate(IntP size) = 0;
	virtual void do_deallocate(void* ptr) = 0;
};

// ==========================================================================

template<typename T>
class MemoryResourceUPtrDeleter
{
public:
	MemoryResourceUPtrDeleter(MemoryResource& memory_resource);

	void operator()(T* ptr) const;

private:
	MemoryResource* memory_resource_{};
};

// --------------------------------------------------------------------------

template<typename T>
MemoryResourceUPtrDeleter<T>::MemoryResourceUPtrDeleter(MemoryResource& memory_resource)
	:
	memory_resource_{&memory_resource}
{}

template<typename T>
void MemoryResourceUPtrDeleter<T>::operator()(T* ptr) const
{
	bstone::destroy_at(ptr);
	memory_resource_->deallocate(ptr);
}

// ==========================================================================

template<typename T>
using MemoryResourceUPtr = std::unique_ptr<T, MemoryResourceUPtrDeleter<T>>;

template<typename T, typename ...TArgs>
auto make_memory_resource_uptr(MemoryResource& memory_resource, TArgs&& ...args)
{
	const auto instance = static_cast<T*>(memory_resource.allocate(static_cast<IntP>(sizeof(T))));

	try
	{
		bstone::construct_at(instance, std::forward<TArgs>(args)...);
	}
	catch (...)
	{
		memory_resource.deallocate(instance);
		throw;
	}

	return MemoryResourceUPtr<T>{instance, MemoryResourceUPtrDeleter<T>{memory_resource}};
}

// ==========================================================================

class NullMemoryResource final : public MemoryResource
{
public:
	NullMemoryResource() = default;
	~NullMemoryResource() override = default;

private:
	void* do_allocate(IntP size) override;
	void do_deallocate(void* ptr) override;
};

// ==========================================================================

class NewDeleteMemoryResource final : public MemoryResource
{
public:
	NewDeleteMemoryResource() = default;
	~NewDeleteMemoryResource() override = default;

private:
	void* do_allocate(IntP size) override;
	void do_deallocate(void* ptr) override;
};

// ==========================================================================

MemoryResource& get_null_memory_resource();
MemoryResource& get_new_delete_memory_resource();
MemoryResource& get_default_memory_resource();

} // namespace bstone

#endif // BSTONE_MEMORY_RESOURCE_INCLUDED
