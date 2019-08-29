/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// Macros to test renderer features (debug builds only).
//


#ifndef BSTONE_RENDERER_TESTS_INCLUDED
#define BSTONE_RENDERER_TESTS_INCLUDED


#ifdef _DEBUG

// Use textures with power-of-two dimensions.
//#define BSTONE_RENDERER_TEST_POT_ONLY

// Use software mipmap generation.
//#define BSTONE_RENDERER_TEST_SW_MIPMAP

// Use software samplers.
//#define BSTONE_RENDERER_TEST_SW_SAMPLER

// Don't use anisotropy filter.
//#define BSTONE_RENDERER_TEST_NO_ANISOTROPY

// Use default framebuffer.
//#define BSTONE_RENDERER_TEST_DEFAULT_FRAMEBUFFER

// Don't use VAO (except if required).
//#define BSTONE_RENDERER_TEST_OGL_NO_VAO

// No swap interval control.
//#define BSTONE_RENDERER_TEST_NO_SWAP_INTERVAL

// No buffer storage.
//#define BSTONE_RENDERER_TEST_OGL_NO_BUFFER_STORAGE

// No direct state access.
//#define BSTONE_RENDERER_TEST_OGL_NO_DSA

#endif // _DEBUG


#endif // !BSTONE_RENDERER_TESTS_INCLUDED
