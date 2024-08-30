/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Debug Bild Tests

#ifndef BSTONE_R3R_TESTS_INCLUDED
#define BSTONE_R3R_TESTS_INCLUDED

#ifndef NDEBUG

// Use textures with power-of-two dimensions.
//#define BSTONE_R3R_TEST_POT_ONLY

// Use software mipmap generation.
//#define BSTONE_R3R_TEST_SW_MIPMAP

// Use software samplers.
//#define BSTONE_R3R_TEST_SW_SAMPLER

// Don't use anisotropy filter.
//#define BSTONE_R3R_TEST_NO_ANISOTROPY

// Use default framebuffer.
//#define BSTONE_R3R_TEST_DEFAULT_FRAMEBUFFER

// Don't use VAO except if mandatory.
//#define BSTONE_R3R_TEST_GL_NO_VAO

// No swap interval control.
//#define BSTONE_R3R_TEST_NO_SWAP_INTERVAL

// No buffer storage.
//#define BSTONE_R3R_TEST_GL_NO_BUFFER_STORAGE

// No direct state access.
//#define BSTONE_R3R_TEST_GL_NO_DSA

// No separate shader objects.
//#define BSTONE_R3R_TEST_GL_NO_SSO

// No OpenGL renderer.
//#define BSTONE_R3R_TEST_NO_GL

// No OpenGL 2.0.
//#define BSTONE_R3R_TEST_NO_GL_2_0

// No OpenGL 3.2 core.
//#define BSTONE_R3R_TEST_NO_GL_3_2_C

// No OpenGL ES 2.0.
//#define BSTONE_R3R_TEST_NO_GLES_2_0

#endif // NDEBUG

#endif // BSTONE_R3R_TESTS_INCLUDED
