/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_ENTRY_POINT_INCLUDED
#define BSTONE_ENTRY_POINT_INCLUDED

#if defined(_WIN32)
	#ifndef BSTONE_ENTRY_POINT_IMPLEMENTATION
		#undef main
		#define main bstone_entry_point
	#endif

	extern int bstone_entry_point(int argc, char** argv);
#endif

#endif // BSTONE_ENTRY_POINT_INCLUDED
