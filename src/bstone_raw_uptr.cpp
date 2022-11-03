/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include "bstone_raw_uptr.h"
#include "bstone_utility.h"

namespace bstone {

RawUPtr::RawUPtr() noexcept = default;

RawUPtr::RawUPtr(void* resource) noexcept
	:
	resource_{resource}
{}

RawUPtr::RawUPtr(RawUPtr&& rhs) noexcept
{
	swap(rhs);
}

RawUPtr& RawUPtr::operator=(std::nullptr_t) noexcept
{
	reset();
	return *this;
}

RawUPtr& RawUPtr::operator=(RawUPtr&& rhs) noexcept
{
	swap(rhs);
	return *this;
}

RawUPtr::~RawUPtr()
{
	deallocate(resource_);
}

void* RawUPtr::get() const noexcept
{
	return resource_;
}

void RawUPtr::reset(void* resource) noexcept
{
	deallocate(resource_);
	resource_ = resource;
}

void RawUPtr::reset() noexcept
{
	reset(nullptr);
}

void RawUPtr::swap(RawUPtr& rhs) noexcept
{
	utility::swap(resource_, rhs.resource_);
}

void RawUPtr::deallocate(void* resource) noexcept
{
	::operator delete(resource);
}

// ==========================================================================

bool operator==(const RawUPtr& lhs, std::nullptr_t) noexcept
{
	return lhs.get() == nullptr;
}

bool operator!=(const RawUPtr& lhs, std::nullptr_t) noexcept
{
	return !(lhs == nullptr);
}

bool operator==(std::nullptr_t, const RawUPtr& rhs) noexcept
{
	return rhs == nullptr;
}

bool operator!=(std::nullptr_t, const RawUPtr& rhs) noexcept
{
	return !(rhs == nullptr);
}

// ==========================================================================

RawUPtr make_raw_uptr(Int size)
{
	assert(size >= 0);
	return RawUPtr{::operator new(size)};
}

} // namespace bstone
