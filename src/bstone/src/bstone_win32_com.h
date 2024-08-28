/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows COM stuff.

#ifndef BSTONE_WIN32_COM_INCLUDED
#define BSTONE_WIN32_COM_INCLUDED

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <memory>

#include <wtypes.h>
#include <oaidl.h>
#include <unknwn.h>

namespace bstone {
namespace win32 {

struct ComScope
{
	ComScope();
	~ComScope();
};

// ==========================================================================

struct IUnknownDeleter
{
	void operator()(IUnknown* ptr) const noexcept;
};

template<typename T>
using ComUPtr = std::unique_ptr<T, IUnknownDeleter>;

// ==========================================================================

class BStr
{
public:
	BStr(const wchar_t* string);
	~BStr();

	operator BSTR() const noexcept;

private:
	BSTR bstr_{};
};

// ==========================================================================

class Variant
{
public:
	Variant() noexcept;
	~Variant();

	VARIANT& get() noexcept;

private:
	VARIANT variant_{};
};

} // namespace win32
} // namespace bstone

#endif // _WIN32

#endif // BSTONE_WIN32_COM_INCLUDED
