/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// C++ standard features.

#if !defined(BSTONE_CXX_INCLUDED)
#define BSTONE_CXX_INCLUDED

#if !defined(BSTONE_CXX_NODISCARD)
	#if __cplusplus == 201703L
		#define BSTONE_CXX_NODISCARD [[nodiscard]]
	#else
		#define BSTONE_CXX_NODISCARD
	#endif
#endif // BSTONE_CXX_NODISCARD

#endif // BSTONE_CXX_INCLUDED
