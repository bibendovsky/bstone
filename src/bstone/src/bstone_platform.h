/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// Custom macros (auto-detected or user-defined):
//    BSTONE_WIN32 - Win32 platform.
//                      Values:
//                          - 0 - not WIN32.
//                          - 1 - WIN32.
//
//    BSTONE_MINGW - MinGW environment.
//                      Values:
//                          - 0 - not MinGW.
//                          - 1 - MinGW.
//
//    BSTONE_POSIX - POSIX compatible platform.
//                      Values:
//                          - 0 - not POSIX.
//                          - 1 - POSIX.
//

#ifndef BSTONE_PLATFORM_INCLUDED
#define BSTONE_PLATFORM_INCLUDED

#if defined(_WIN32)

	#ifndef BSTONE_WIN32
		#define BSTONE_WIN32 1
	#endif

#else

	#ifndef BSTONE_POSIX
		#define BSTONE_POSIX 1
	#endif

#endif // _WIN32

#if defined(__MINGW32__)
	#define BSTONE_MINGW 1
#endif


#ifndef BSTONE_WIN32
	#define BSTONE_WIN32 0
#endif

#ifndef BSTONE_MINGW
	#define BSTONE_MINGW 0
#endif

#ifndef BSTONE_POSIX
	#define BSTONE_POSIX 0
#endif

// ==========================================================================

#if (BSTONE_WIN32 + BSTONE_POSIX) != 1
	#error Expected one platform.
#endif

// ==========================================================================

#ifndef BSTONE_EXPORT
	#if defined(_WIN32)
		#define BSTONE_EXPORT __declspec(dllexport)
	#elif __GNUC__ >= 4
		#define BSTONE_EXPORT __attribute__ ((visibility("default")))
	#elif defined(__clang__)
		#if __has_declspec_attribute(dllexport)
			#define BSTONE_EXPORT __declspec(dllexport)
		#endif
	#endif
#endif

#ifndef BSTONE_EXPORT
	#define BSTONE_EXPORT
#endif

#endif // BSTONE_PLATFORM_INCLUDED
