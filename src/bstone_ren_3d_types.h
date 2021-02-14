/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// 3D renderer's types.
//


#ifndef BSTONE_REN_3D_TYPES_INCLUDED
#define BSTONE_REN_3D_TYPES_INCLUDED


#include <array>
#include <memory>
#include <string>

#include "bstone_rgb8.h"


namespace bstone
{


using Ren3dVec2 = std::array<float, 2>;
using Ren3dVec4 = std::array<float, 4>;
using Ren3dMat4 = std::array<float, 16>;


enum class Ren3dKind
{
	gl_2_0,
	gl_3_2_core,

	gles_2_0,
}; // Ren3dKind

enum class Ren3dPrimitiveTopology
{
	none,

	point_list,

	line_list,
	line_strip,

	triangle_list,
	triangle_strip,
}; // Ren3dPrimitiveTopology

enum class Ren3dCullingFace
{
	counter_clockwise,
	clockwise,
}; // Ren3dCullingFace

enum class Ren3dCullingMode
{
	back,
	front,
	both,
}; // Ren3dCullingMode

enum class Ren3dBlendingFactor
{
	zero,
	one,
	src_color,
	src_alpha,
	one_minus_src_alpha,
}; // Ren3dBlendingFactor

enum class Ren3dAaKind
{
	none,
	ms,
}; // Ren3dAaKind

enum class Ren3dFilterKind
{
	nearest,
	linear,
}; // Ren3dFilterKind

enum class Ren3dMipmapMode
{
	none,
	nearest,
	linear,
}; // Ren3dMipmapMode

enum class Ren3dAddressMode
{
	clamp,
	repeat,
}; // Ren3dAddressMode

enum class Ren3dTextureAxis
{
	u,
	v,
}; // Ren3dTextureAxis

enum class Ren3dPixelFormat
{
	none,
	rgba_8_unorm,
}; // Ren3dPixelFormat


struct Ren3dViewport
{
	int x_;
	int y_;
	int width_;
	int height_;

	float min_depth_;
	float max_depth_;
}; // Ren3dViewport

struct Ren3dScissorBox
{
	int x_;
	int y_;
	int width_;
	int height_;
}; // Ren3dScissorBox

struct Ren3dBlendingFunc
{
	Ren3dBlendingFactor src_factor_;
	Ren3dBlendingFactor dst_factor_;
}; // Ren3dBlendingFunc

struct Ren3dClearParam
{
	Rgba8 color_;
}; // Ren3dClearParam


// ==========================================================================
// Ren3dDeviceFeatures
//

struct Ren3dDeviceFeatures
{
	bool is_vsync_available_;
	bool is_vsync_requires_restart_;

	int max_texture_dimension_;

	int max_viewport_width_;
	int max_viewport_height_;

	bool is_anisotropy_available_;
	int max_anisotropy_degree_;

	bool is_npot_available_;

	bool is_mipmap_available_;

	bool is_sampler_available_;

	bool is_msaa_available_;
	bool is_msaa_render_to_window_;
	bool is_msaa_requires_restart_;
	int max_msaa_degree_;

	int max_vertex_input_locations_;
}; // Ren3dDeviceFeatures

//
// Ren3dDeviceFeatures
// ==========================================================================


// ==========================================================================
// Ren3dDeviceInfo
//

struct Ren3dDeviceInfo
{
	std::string name_;
	std::string vendor_;
	std::string version_;
}; // Ren3dDeviceInfo

//
// Ren3dDeviceInfo
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_TYPES_INCLUDED
