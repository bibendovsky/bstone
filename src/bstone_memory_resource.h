/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_MEMORY_RESOURCE_INCLUDED)
#define BSTONE_MEMORY_RESOURCE_INCLUDED

#include <cstddef>

namespace bstone {

class MemoryResource
{
public:
	MemoryResource() = default;
	virtual ~MemoryResource() = default;

	void* allocate(std::size_t size);
	void deallocate(void* ptr);

private:
	virtual void* do_allocate(std::size_t size) = 0;
	virtual void do_deallocate(void* ptr) = 0;
};

// ==========================================================================

class NewDeleteMemoryResource final : public MemoryResource
{
public:
	NewDeleteMemoryResource() = default;
	~NewDeleteMemoryResource() override = default;

private:
	void* do_allocate(std::size_t size) override;
	void do_deallocate(void* ptr) override;
};

// ==========================================================================

MemoryResource& get_default_memory_resource() noexcept;

} // namespace bstone

#endif // BSTONE_MEMORY_RESOURCE_INCLUDED
