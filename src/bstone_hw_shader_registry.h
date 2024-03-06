/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Shader registry.
//


#ifndef BSTONE_HW_SHADER_REGISTRY_INCLUDED
#define BSTONE_HW_SHADER_REGISTRY_INCLUDED


#include <string>

#include "bstone_r3r_shader.h"
#include "bstone_r3r_types.h"


namespace bstone
{


class HwShaderRegistry
{
public:
	static int get_a_position_location();

	static int get_a_color_location();

	static int get_a_tx_coords_location();


	static const char* get_a_position_name();
	static const char* get_a_color_name();
	static const char* get_a_tx_coords_name();


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


	static const R3rShaderSource& get_fragment(
		const R3rType renderer_type);

	static const R3rShaderSource& get_vertex(
		const R3rType renderer_type);


private:
	static const R3rShaderSource& get_empty();

	static const R3rShaderSource& get_fragment_gl();
	static const R3rShaderSource& get_vertex_gl();
}; // HwShaderRegistry


} // bstone


#endif // !BSTONE_HW_SHADER_REGISTRY_INCLUDED
