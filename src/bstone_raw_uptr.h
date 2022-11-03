/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_RAW_UPTR_INCLUDED
#define BSTONE_RAW_UPTR_INCLUDED

#include <cstddef>
#include "bstone_int.h"

namespace bstone {

class RawUPtr
{
public:
	RawUPtr() noexcept;
	explicit RawUPtr(void* resource) noexcept;
	RawUPtr(const RawUPtr&) = delete;
	RawUPtr(RawUPtr&& rhs) noexcept;

	RawUPtr& operator=(std::nullptr_t) noexcept;
	RawUPtr& operator=(const RawUPtr&) = delete;
	RawUPtr& operator=(RawUPtr&& rhs) noexcept;

	~RawUPtr();

	void* get() const noexcept;

	template<typename T>
	T get() const noexcept
	{
		return static_cast<T>(get());
	}

	void reset(void* resource) noexcept;
	void reset() noexcept;
	void swap(RawUPtr& rhs) noexcept;

private:
	void* resource_{};

	static void deallocate(void* resource) noexcept;
};

// ==========================================================================

bool operator==(const RawUPtr& lhs, std::nullptr_t) noexcept;
bool operator!=(const RawUPtr& lhs, std::nullptr_t) noexcept;

bool operator==(std::nullptr_t, const RawUPtr& rhs) noexcept;
bool operator!=(std::nullptr_t, const RawUPtr& rhs) noexcept;

// ==========================================================================

RawUPtr make_raw_uptr(Int size);

} // namespace bstone

#endif // !BSTONE_RAW_UPTR_INCLUDED
