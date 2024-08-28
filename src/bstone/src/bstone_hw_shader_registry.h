/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

/*
Shader registry.

Notes:
  - See CalcHeight and ScalePost in 3d_draw.cpp for shading calculation.
*/

#ifndef BSTONE_HW_SHADER_REGISTRY_INCLUDED
#define BSTONE_HW_SHADER_REGISTRY_INCLUDED

#include "bstone_r3r_shader.h"
#include "bstone_r3r_types.h"

namespace bstone {

class HwShaderRegistry
{
public:
	static int get_a_position_location() noexcept;
	static int get_a_color_location() noexcept;
	static int get_a_tx_coords_location() noexcept;

	static const char* get_a_position_name() noexcept;
	static const char* get_a_color_name() noexcept;
	static const char* get_a_tx_coords_name() noexcept;

	static const char* get_u_model_mat_name() noexcept;
	static const char* get_u_view_mat_name() noexcept;
	static const char* get_u_projection_mat_name() noexcept;

	static const char* get_u_sampler_name() noexcept;

	static const char* get_u_shading_mode_name() noexcept;
	static const char* get_u_shade_max_name() noexcept;
	static const char* get_u_normal_shade_name() noexcept;
	static const char* get_u_height_numerator_name() noexcept;
	static const char* get_u_extra_lighting_name() noexcept;
	static const char* get_u_view_direction_name() noexcept;
	static const char* get_u_view_position_name() noexcept;

	static const R3rShaderSource& get_fragment(R3rType renderer_type) noexcept;
	static const R3rShaderSource& get_vertex(R3rType renderer_type) noexcept;

private:
	static R3rShaderSource make_r3r_shader_source(const char* source) noexcept;

	static const R3rShaderSource& get_empty() noexcept;

	static const R3rShaderSource& get_fragment_gl() noexcept;
	static const R3rShaderSource& get_vertex_gl() noexcept;
};

} // namespace bstone

#endif // BSTONE_HW_SHADER_REGISTRY_INCLUDED
