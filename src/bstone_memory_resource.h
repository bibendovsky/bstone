/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_MEMORY_RESOURCE_INCLUDED)
#define BSTONE_MEMORY_RESOURCE_INCLUDED

#include "bstone_int.h"

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
