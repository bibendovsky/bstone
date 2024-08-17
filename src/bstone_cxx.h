/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// C++ standard features.

#ifndef BSTONE_CXX_INCLUDED
#define BSTONE_CXX_INCLUDED

#ifndef BSTONE_CXX_NODISCARD
	#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
		#define BSTONE_CXX_NODISCARD [[nodiscard]]
	#else
		#define BSTONE_CXX_NODISCARD
	#endif
#endif // BSTONE_CXX_NODISCARD

#endif // BSTONE_CXX_INCLUDED
