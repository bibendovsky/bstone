/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows COM stuff.

#ifdef _WIN32

#include "bstone_win32_com.h"

#include <cassert>

#include "bstone_exception.h"

namespace bstone {
namespace win32 {

ComScope::ComScope()
{
	const auto hresult = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

	if (FAILED(hresult))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to initialize COM library.");
	}
}

ComScope::~ComScope()
{
	CoUninitialize();
}

// ==========================================================================

void IUnknownDeleter::operator()(IUnknown* ptr) const noexcept
{
	ptr->Release();
}

// ==========================================================================

BStr::BStr(const wchar_t* string)
	:
	bstr_{SysAllocString(string)}
{}

BStr::~BStr()
{
	SysFreeString(bstr_);
}

BStr::operator BSTR() const noexcept
{
	return bstr_;
}

// ==========================================================================

Variant::Variant() noexcept
{
	VariantInit(&variant_);
}

Variant::~Variant()
{
	VariantClear(&variant_);
}

VARIANT& Variant::get() noexcept
{
	return variant_;
}

} // namespace win32
} // namespace bstone

#endif // _WIN32
