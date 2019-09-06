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
// Shader registry.
//


#ifndef BSTONE_RENDERER_SHADER_REGISTRY_INCLUDED
#define BSTONE_RENDERER_SHADER_REGISTRY_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{


class RendererShaderRegistry final
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

	static const std::string& get_u_texture_mat_name();


	static const std::string& get_u_sampler_name();


	static const std::string& get_u_shading_mode_name();

	static const std::string& get_u_shade_max_name();

	static const std::string& get_u_normal_shade_name();

	static const std::string& get_u_height_numerator_name();

	static const std::string& get_u_extra_lighting_name();

	static const std::string& get_u_view_direction_name();

	static const std::string& get_u_view_position_name();


	static const RendererShaderSource& get_fragment(
		const RendererKind renderer_kind);

	static const RendererShaderSource& get_vertex(
		const RendererKind renderer_kind);


private:
	static const RendererShaderSource& get_empty();

	static const RendererShaderSource& get_fragment_ogl_2();

	static const RendererShaderSource& get_vertex_ogl_2();

	static const RendererShaderSource& get_fragment_ogl_3_2_core();

	static const RendererShaderSource& get_vertex_ogl_3_2_core();
}; // RendererShaderRegistry


} // bstone


#endif // !BSTONE_RENDERER_SHADER_REGISTRY_INCLUDED
