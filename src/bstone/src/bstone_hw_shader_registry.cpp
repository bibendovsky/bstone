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

#include "bstone_hw_shader_registry.h"

#include "bstone_char_traits.h"

namespace bstone {

int HwShaderRegistry::get_a_position_location() noexcept
{
	return 0;
}

int HwShaderRegistry::get_a_color_location() noexcept
{
	return 1;
}

int HwShaderRegistry::get_a_tx_coords_location() noexcept
{
	return 2;
}

const char* HwShaderRegistry::get_a_position_name() noexcept
{
	return "a_position";
}

const char* HwShaderRegistry::get_a_color_name() noexcept
{
	return "a_color";
}

const char* HwShaderRegistry::get_a_tx_coords_name() noexcept
{
	return "a_tx_coords";
}

const char* HwShaderRegistry::get_u_model_mat_name() noexcept
{
	return "u_model_mat";
}

const char* HwShaderRegistry::get_u_view_mat_name() noexcept
{
	return "u_view_mat";
}

const char* HwShaderRegistry::get_u_projection_mat_name() noexcept
{
	return "u_projection_mat";
}

const char* HwShaderRegistry::get_u_sampler_name() noexcept
{
	return "u_sampler";
}

const char* HwShaderRegistry::get_u_shading_mode_name() noexcept
{
	return "u_shading_mode";
}

const char* HwShaderRegistry::get_u_shade_max_name() noexcept
{
	return "u_shade_max";
}

const char* HwShaderRegistry::get_u_normal_shade_name() noexcept
{
	return "u_normal_shade";
}

const char* HwShaderRegistry::get_u_height_numerator_name() noexcept
{
	return "u_height_numerator";
}

const char* HwShaderRegistry::get_u_extra_lighting_name() noexcept
{
	return "u_extra_lighting";
}

const char* HwShaderRegistry::get_u_view_direction_name() noexcept
{
	return "u_view_direction";
}

const char* HwShaderRegistry::get_u_view_position_name() noexcept
{
	return "u_view_position";
}

const R3rShaderSource& HwShaderRegistry::get_fragment(R3rType renderer_type) noexcept
{
	switch (renderer_type)
	{
		case R3rType::gl_2_0:
		case R3rType::gl_3_2_core:
		case R3rType::gles_2_0:
			return get_fragment_gl();

		default: return get_empty();
	}
}

const R3rShaderSource& HwShaderRegistry::get_vertex(R3rType renderer_type) noexcept
{
	switch (renderer_type)
	{
		case R3rType::gl_2_0:
		case R3rType::gl_3_2_core:
		case R3rType::gles_2_0:
			return get_vertex_gl();

		default: return get_empty();
	}
}

R3rShaderSource HwShaderRegistry::make_r3r_shader_source(const char* source) noexcept
{
	auto result = R3rShaderSource{};
	result.data = source;
	result.size = static_cast<int>(char_traits::get_size(source));
	return result;
}

const R3rShaderSource& HwShaderRegistry::get_empty() noexcept
{
	static const auto result = R3rShaderSource{};
	return result;
}

const R3rShaderSource& HwShaderRegistry::get_fragment_gl() noexcept
{
	static const auto result = make_r3r_shader_source(
R"FRAGMENT_SHADER(
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D u_sampler;

// 0 - none
// 1 - vanilla
uniform int u_shading_mode;

uniform float u_shade_max;
uniform float u_normal_shade;
uniform float u_height_numerator;
uniform float u_extra_lighting;

uniform vec2 u_view_direction;
uniform vec2 u_view_position;

varying vec2 o_position;
varying vec4 o_color;
varying vec2 o_tx_coords;

float calculate_no_shading()
{
	return 0.0;
}

float calculate_vanilla_shading()
{
	if (u_shade_max <= 0.0)
	{
		return 0.0;
	}

	const float min_nx = 0.00001;
	const float min_height = 8.0;
	const float max_index = 65.0;


	float view_cos = u_view_direction.x;
	float view_sin = -u_view_direction.y;

	float gx = o_position.x - u_view_position.x;
	float gxt = gx * view_cos;

	float gy = o_position.y - u_view_position.y;
	float gyt = gy * view_sin;

	float nx = max(gxt - gyt, min_nx);

	float height = max((256.0 * u_height_numerator) / nx, min_height);
	height /= 8.0;

	float index = u_shade_max - ((63.0 * height) / u_normal_shade) + u_extra_lighting;
	index = clamp(index, 0.0, 63.0);

	float shading_weight = index / max_index;

	return shading_weight;
}

float calculate_shading_weight()
{
	if (u_shading_mode == 0)
	{
		return calculate_no_shading();
	}
	else if (u_shading_mode == 1)
	{
		return calculate_vanilla_shading();
	}
	else
	{
		return 0.0;
	}
}

void main()
{
	float shading_weight = calculate_shading_weight();


	float alpha;

	gl_FragColor = o_color * texture2D(u_sampler, o_tx_coords);
	alpha = gl_FragColor.a;
	gl_FragColor *= 1.0 - shading_weight;
	gl_FragColor.a = alpha;
}
)FRAGMENT_SHADER");

	return result;
}

const R3rShaderSource& HwShaderRegistry::get_vertex_gl() noexcept
{
	static const auto result = make_r3r_shader_source(
R"VERTEX_SHADER(
attribute vec3 a_position;
attribute vec4 a_color;
attribute vec2 a_tx_coords;

uniform mat4 u_model_mat;
uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;

varying vec2 o_position;
varying vec4 o_color;
varying vec2 o_tx_coords;

void main()
{
	vec4 position = u_model_mat * vec4(a_position, 1.0);

	o_position = position.xy;
	o_color = a_color;
	o_tx_coords = vec2(a_tx_coords.x, 1.0 - a_tx_coords.y);

	gl_Position = u_projection_mat * u_view_mat * position;
}
)VERTEX_SHADER");

	return result;
}

} // namespace bstone
