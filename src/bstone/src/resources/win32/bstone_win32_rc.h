/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#define VS_VERSION_INFO 1

#ifndef VS_FFI_FILEFLAGSMASK
#define VS_FFI_FILEFLAGSMASK 0x0000003FL
#endif // !VS_FFI_FILEFLAGSMASK

#ifdef NDEBUG
#define VER_DBG 0
#else // NDEBUG
#define VER_DBG 1
#endif // NDEBUG

#ifndef VOS_NT
#define VOS_NT 0x00040000L
#endif // !VOS_NT

#ifndef VFT_DRV
#define VFT_DRV 0x00000003L
#endif // !VFT_DRV

#ifndef VFT2_DRV_SYSTEM
#define VFT2_DRV_SYSTEM 0x00000007L
#endif // !VFT2_DRV_SYSTEM

#ifndef RT_MANIFEST
#define RT_MANIFEST 24
#endif // !RT_MANIFEST
