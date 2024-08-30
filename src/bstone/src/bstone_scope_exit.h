/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A general-purpose scope guard intended to call its exit function when a scope is exited.

#ifndef BSTONE_SCOPE_EXIT_INCLUDED
#define BSTONE_SCOPE_EXIT_INCLUDED

#include <utility>

#include "bstone_utility.h"

namespace bstone {

template<typename TFunctor>
class ScopeExit
{
public:
	ScopeExit() = delete;

	explicit ScopeExit(TFunctor&& functor) noexcept
		:
		functor_{std::move(functor)},
		is_released_{}
	{}

	ScopeExit(const ScopeExit& rhs) = delete;

	ScopeExit(ScopeExit&& rhs) noexcept
		:
		functor_{std::move(rhs.functor_)},
		is_released_{rhs.is_released_}
	{
		rhs.is_released_ = true;
	}

	ScopeExit& operator=(const ScopeExit& rhs) = delete;

	~ScopeExit()
	{
		if (is_released_)
		{
			return;
		}

		functor_();
	}

	void release() noexcept
	{
		is_released_ = true;
	}

	void swap(ScopeExit& rhs) noexcept
	{
		bstone::swop(functor_, rhs.functor_);
		bstone::swop(is_released_, rhs.is_released_);
	}

private:
	TFunctor functor_;
	bool is_released_;
};

// ==========================================================================

template<typename TFunctor>
auto make_scope_exit(TFunctor&& functor) noexcept
{
	return ScopeExit<TFunctor>{std::forward<TFunctor>(functor)};
}

} // namespace bstone

#endif // BSTONE_SCOPE_EXIT_INCLUDED
