/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Texture lock

#ifndef BSTONE_TEXTURE_LOCK_INCLUDED
#define BSTONE_TEXTURE_LOCK_INCLUDED

#include <memory>

namespace bstone::sys {

class TextureLock
{
public:
	TextureLock() = default;
	virtual ~TextureLock() = default;

	void* get_pixels() const;

	template<typename T>
	T get_pixels() const
	{
		return static_cast<T>(get_pixels());
	}

	int get_pitch() const;

private:
	virtual void* do_get_pixels() const = 0;
	virtual int do_get_pitch() const = 0;
};

// ======================================

using TextureLockUPtr = std::unique_ptr<TextureLock>;

} // namespace bstone::sys

#endif // BSTONE_TEXTURE_LOCK_INCLUDED
