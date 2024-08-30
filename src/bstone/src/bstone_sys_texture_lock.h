/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_TEXTURE_LOCK_INCLUDED
#define BSTONE_TEXTURE_LOCK_INCLUDED

#include <memory>

namespace bstone {
namespace sys {

class TextureLock
{
public:
	TextureLock();
	virtual ~TextureLock();

	void* get_pixels() const noexcept;

	template<typename T>
	T get_pixels() const noexcept
	{
		return static_cast<T>(get_pixels());
	}

	int get_pitch() const noexcept;

private:
	virtual void* do_get_pixels() const noexcept = 0;
	virtual int do_get_pitch() const noexcept = 0;
};

// ==========================================================================

using TextureLockUPtr = std::unique_ptr<TextureLock>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_TEXTURE_LOCK_INCLUDED
