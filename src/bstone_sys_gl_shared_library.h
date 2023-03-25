/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_GL_SHARED_LIBRARY_INCLUDED)
#define BSTONE_SYS_GL_SHARED_LIBRARY_INCLUDED

#include <memory>

namespace bstone {
namespace sys {

class GlSharedLibrary
{
public:
	GlSharedLibrary() = default;
	virtual ~GlSharedLibrary() = default;

	void* find_symbol(const char* name) noexcept;

	template<typename T>
	T find_symbol(const char* name) noexcept
	{
		return reinterpret_cast<T>(find_symbol(name));
	}

private:
	virtual void* do_find_symbol(const char* name) noexcept = 0;
};

// ==========================================================================

using GlSharedLibraryUPtr = std::unique_ptr<GlSharedLibrary>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_SHARED_LIBRARY_INCLUDED
