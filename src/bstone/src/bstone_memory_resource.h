/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_MEMORY_RESOURCE_INCLUDED
#define BSTONE_MEMORY_RESOURCE_INCLUDED

#include <cstdint>

#include <memory>

#include "bstone_cxx.h"
#include "bstone_memory.h"

namespace bstone {

class MemoryResource
{
public:
	MemoryResource() noexcept;
	virtual ~MemoryResource();

	template<typename T>
	BSTONE_CXX_NODISCARD T* allocate(std::intptr_t count);

	BSTONE_CXX_NODISCARD void* allocate(std::intptr_t size);
	void deallocate(void* ptr) noexcept;

private:
	BSTONE_CXX_NODISCARD virtual void* do_allocate(std::intptr_t size) = 0;
	virtual void do_deallocate(void* ptr) noexcept = 0;
};

// --------------------------------------------------------------------------

template<typename T>
BSTONE_CXX_NODISCARD T* MemoryResource::allocate(std::intptr_t count)
{
	static_assert(sizeof(T) <= PTRDIFF_MAX, "Size of type too big.");
	constexpr auto item_size = static_cast<std::intptr_t>(sizeof(T));
	const auto size = item_size * count;
	return static_cast<T*>(allocate(size));
}

// ==========================================================================

class MemoryResourceUPtrDeleterBase
{
public:
	explicit MemoryResourceUPtrDeleterBase(MemoryResource& memory_resource) noexcept;

	MemoryResource& get_memory_resource() const noexcept;

	template<typename T>
	void invoke(T* ptr) const noexcept;

private:
	MemoryResource* memory_resource_{};
};

// --------------------------------------------------------------------------

template<typename T>
void MemoryResourceUPtrDeleterBase::invoke(T* ptr) const noexcept
{
	bstone::destroy_at(ptr);
	memory_resource_->deallocate(ptr);
}

// ==========================================================================

template<typename T>
class MemoryResourceUPtrDeleter final : public MemoryResourceUPtrDeleterBase
{
public:
	MemoryResourceUPtrDeleter(MemoryResource& memory_resource) noexcept;

	void operator()(T* ptr) const noexcept;
};

// --------------------------------------------------------------------------

template<typename T>
MemoryResourceUPtrDeleter<T>::MemoryResourceUPtrDeleter(MemoryResource& memory_resource) noexcept
	:
	MemoryResourceUPtrDeleterBase{memory_resource}
{}

template<typename T>
void MemoryResourceUPtrDeleter<T>::operator()(T* ptr) const noexcept
{
	invoke(ptr);
}

// ==========================================================================

template<typename T>
class MemoryResourceUPtrDeleter<T[]> final : public MemoryResourceUPtrDeleterBase
{
public:
	MemoryResourceUPtrDeleter(MemoryResource& memory_resource) noexcept;

	void operator()(T* ptr) const noexcept;
};

// --------------------------------------------------------------------------

template<typename T>
MemoryResourceUPtrDeleter<T[]>::MemoryResourceUPtrDeleter(MemoryResource& memory_resource) noexcept
	:
	MemoryResourceUPtrDeleterBase{memory_resource}
{}

template<typename T>
void MemoryResourceUPtrDeleter<T[]>::operator()(T* ptr) const noexcept
{
	invoke(ptr);
}

// ==========================================================================

template<typename T>
using MemoryResourceUPtr = std::unique_ptr<T, MemoryResourceUPtrDeleter<T>>;

template<typename T, typename ...TArgs>
auto make_memory_resource_uptr(MemoryResource& memory_resource, TArgs&& ...args)
{
	const auto instance = static_cast<T*>(memory_resource.allocate(
		static_cast<std::intptr_t>(sizeof(T))));

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
	NullMemoryResource() noexcept;
	~NullMemoryResource() override;

private:
	BSTONE_CXX_NODISCARD void* do_allocate(std::intptr_t size) override;
	void do_deallocate(void* ptr) noexcept override;
};

// ==========================================================================

class NewDeleteMemoryResource final : public MemoryResource
{
public:
	NewDeleteMemoryResource() noexcept;
	~NewDeleteMemoryResource() override;

private:
	BSTONE_CXX_NODISCARD void* do_allocate(std::intptr_t size) override;
	void do_deallocate(void* ptr) noexcept override;
};

// ==========================================================================

MemoryResource& get_null_memory_resource() noexcept;
MemoryResource& get_new_delete_memory_resource() noexcept;
MemoryResource& get_default_memory_resource() noexcept;

} // namespace bstone

#endif // BSTONE_MEMORY_RESOURCE_INCLUDED
