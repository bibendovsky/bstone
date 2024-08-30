/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Types

#ifndef BSTONE_R3R_TYPES_INCLUDED
#define BSTONE_R3R_TYPES_INCLUDED

#include <array>
#include <memory>
#include "bstone_string_view.h"
#include "bstone_sys_color.h"

namespace bstone {

using R3rVec2 = std::array<float, 2>;
using R3rVec4 = std::array<float, 4>;
using R3rMat4 = std::array<float, 16>;

// ==========================================================================

enum class R3rType
{
	none = 0,

	gl_2_0,
	gl_3_2_core,

	gles_2_0,
};

enum class R3rPrimitiveType
{
	none = 0,

	point_list,

	line_list,
	line_strip,

	triangle_list,
	triangle_strip,
};

enum class R3rCullingFace
{
	none = 0,
	counter_clockwise,
	clockwise,
};

enum class R3rCullingMode
{
	none = 0,
	back,
	front,
	both,
};

enum class R3rBlendingFactor
{
	none = 0,
	zero,
	one,
	src_color,
	src_alpha,
	one_minus_src_alpha,
};

enum class R3rAaType
{
	none = 0,
	ms,
};

enum class R3rFilterType
{
	none = 0,
	nearest,
	linear,
};

enum class R3rMipmapMode
{
	none = 0,
	nearest,
	linear,
};

enum class R3rAddressMode
{
	none = 0,
	clamp,
	repeat,
};

enum class R3rTextureAxis
{
	none = 0,
	u,
	v,
};

enum class R3rPixelFormat
{
	none = 0,
	rgba_8_unorm,
};

// ==========================================================================

struct R3rViewport
{
	int x;
	int y;
	int width;
	int height;

	float min_depth;
	float max_depth;
};

struct R3rScissorBox
{
	int x;
	int y;
	int width;
	int height;
};

struct R3rBlendingFunc
{
	R3rBlendingFactor src_factor;
	R3rBlendingFactor dst_factor;
};

// ==========================================================================

struct R3rDeviceFeatures
{
	bool is_vsync_available;
	bool is_vsync_requires_restart;

	int max_texture_dimension;

	int max_viewport_width;
	int max_viewport_height;

	bool is_anisotropy_available;
	int max_anisotropy_degree;

	bool is_npot_available;

	bool is_mipmap_available;

	bool is_sampler_available;

	bool is_msaa_available;
	bool is_msaa_render_to_window;
	bool is_msaa_requires_restart;
	int max_msaa_degree;

	int max_vertex_input_locations;
};

// ==========================================================================

struct R3rDeviceInfo
{
	StringView name;
	StringView vendor;
	StringView version;
};

// ==========================================================================

struct R3rClearParam
{
	sys::Color color;
};

} // namespace bstone

#endif // BSTONE_R3R_TYPES_INCLUDED
