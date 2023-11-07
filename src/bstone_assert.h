/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Assertion facility.

/*
Macros:
  - BSTONE_NO_ASSERT
    Skips assertion if defined.

  - BSTONE_ASSERT
    Asserts if macros `NDEBUG` and `BSTONE_NO_ASSERT` are not defined or do nothing otherwise.
*/

#if !defined(BSTONE_ASSERT_INCLUDED)
#define BSTONE_ASSERT_INCLUDED

#if !defined(NDEBUG) && !defined(BSTONE_NO_ASSERT)
	#include <cassert>

	#if !defined(BSTONE_ASSERT)
		#define BSTONE_ASSERT(x) assert(x)
	#endif
#else
	#if !defined(BSTONE_ASSERT)
		#define BSTONE_ASSERT(x)
	#endif
#endif

#endif // BSTONE_ASSERT_INCLUDED
