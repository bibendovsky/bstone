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
// 3D renderer.
//


#ifndef BSTONE_REN_3D_INCLUDED
#define BSTONE_REN_3D_INCLUDED


#include <memory>
#include <string>

#include "bstone_rect_2d.h"
#include "bstone_ren_3d_buffer.h"
#include "bstone_ren_3d_texture_2d.h"
#include "bstone_ren_3d_types.h"
#include "bstone_ren_3d_sampler.h"
#include "bstone_ren_3d_shader.h"
#include "bstone_ren_3d_shader_stage.h"
#include "bstone_ren_3d_vertex_input.h"


namespace bstone
{


class Ren3dCmdBuffer;
using Ren3dCmdBufferPtr = Ren3dCmdBuffer*;


struct Ren3dCreateWindowParam
{
	bool is_visible_;
	bool is_borderless_;
	bool is_fake_fullscreen_;
	bool is_positioned_;

	Rect2d rect_2d_;

	std::string title_;
}; // Ren3dCreateWindowParam

struct Ren3dSetWindowModeParam
{
	bool is_windowed_;
	bool is_positioned_;

	Rect2d rect_2d_;
}; // Ren3dSetWindowModeParam

struct Ren3dCreateParam
{
	Ren3dKind renderer_kind_;
	Ren3dCreateWindowParam window_;

	Ren3dAaKind aa_kind_;
	int aa_value_;

	bool is_vsync_;
}; // Ren3dCreateParam

struct Ren3dDrawIndexedParam
{
	// Primitive topology.
	Ren3dPrimitiveTopology primitive_topology_;

	// Vertex count.
	int vertex_count_;

	// Size of the element in bytes.
	int index_byte_depth_;

	// Offset in bytes from the start of index buffer.
	int index_buffer_offset_;

	// Number of the index to draw from.
	int index_offset_;
}; // Ren3dDrawIndexedParam


class Ren3d
{
protected:
	Ren3d() = default;

	virtual ~Ren3d() = default;


public:
	virtual Ren3dKind get_kind() const noexcept = 0;

	virtual const std::string& get_name() const noexcept = 0;

	virtual const std::string& get_description() const noexcept = 0;


	virtual const Ren3dDeviceFeatures& get_device_features() const noexcept = 0;

	virtual const Ren3dDeviceInfo& get_device_info() const noexcept = 0;


	virtual void set_window_mode(
		const Ren3dSetWindowModeParam& param) = 0;

	virtual void set_window_title(
		const std::string& title) = 0;

	virtual void show_window(
		const bool is_visible) = 0;


	virtual bool get_vsync() const noexcept = 0;

	virtual void enable_vsync(
		const bool is_enabled) = 0;


	virtual void set_anti_aliasing(
		const Ren3dAaKind aa_kind,
		const int aa_value) = 0;


	virtual void read_pixels_rgb_888(
		void* buffer,
		bool& is_flipped_vertically) = 0;


	virtual void present() = 0;


	virtual Ren3dBufferUPtr create_buffer(
		const Ren3dCreateBufferParam& param) = 0;

	virtual Ren3dTexture2dUPtr create_texture_2d(
		const Ren3dCreateTexture2dParam& param) = 0;

	virtual Ren3dSamplerUPtr create_sampler(
		const Ren3dCreateSamplerParam& param) = 0;

	virtual Ren3dVertexInputUPtr create_vertex_input(
		const Ren3dCreateVertexInputParam& param) = 0;

	virtual Ren3dShaderUPtr create_shader(
		const Ren3dCreateShaderParam& param) = 0;

	virtual Ren3dShaderStageUPtr create_shader_stage(
		const Ren3dCreateShaderStageParam& param) = 0;


	virtual void submit_commands(
		Ren3dCmdBufferPtr* const command_buffers,
		const int command_buffer_count) = 0;
}; // Ren3d

using Ren3dPtr = Ren3d*;


} // bstone


#endif // !BSTONE_REN_3D_INCLUDED
