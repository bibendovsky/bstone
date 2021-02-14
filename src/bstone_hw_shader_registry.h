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
// Shader registry.
//


#ifndef BSTONE_HW_SHADER_REGISTRY_INCLUDED
#define BSTONE_HW_SHADER_REGISTRY_INCLUDED


#include <string>

#include "bstone_ren_3d_shader.h"
#include "bstone_ren_3d_types.h"


namespace bstone
{


class HwShaderRegistry
{
public:
	static int get_a_position_location();

	static int get_a_color_location();

	static int get_a_tx_coords_location();


	static const std::string& get_a_position_name();

	static const std::string& get_a_color_name();

	static const std::string& get_a_tx_coords_name();


	static const std::string& get_u_model_mat_name();

	static const std::string& get_u_view_mat_name();

	static const std::string& get_u_projection_mat_name();


	static const std::string& get_u_sampler_name();


	static const std::string& get_u_shading_mode_name();

	static const std::string& get_u_shade_max_name();

	static const std::string& get_u_normal_shade_name();

	static const std::string& get_u_height_numerator_name();

	static const std::string& get_u_extra_lighting_name();

	static const std::string& get_u_view_direction_name();

	static const std::string& get_u_view_position_name();


	static const Ren3dShaderSource& get_fragment(
		const Ren3dKind renderer_kind);

	static const Ren3dShaderSource& get_vertex(
		const Ren3dKind renderer_kind);


private:
	static const Ren3dShaderSource& get_empty();

	static const Ren3dShaderSource& get_fragment_gl_2();

	static const Ren3dShaderSource& get_vertex_gl_2();

	static const Ren3dShaderSource& get_fragment_gl_3_2_core();

	static const Ren3dShaderSource& get_vertex_gl_3_2_core();

	static const Ren3dShaderSource& get_fragment_gles_2_0();

	static const Ren3dShaderSource& get_vertex_gles_2_0();
}; // HwShaderRegistry


} // bstone


#endif // !BSTONE_HW_SHADER_REGISTRY_INCLUDED
