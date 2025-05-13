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
#include <stdint.h>
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

		case R3rType::vulkan:
			return get_fragment_vk();

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

		case R3rType::vulkan:
			return get_vertex_vk();

		default: return get_empty();
	}
}

const Span<const R3rShaderVarInfo> HwShaderRegistry::get_shader_var_infos()
{
	static const R3rShaderVarInfo result[] =
	{
		{
			"a_position", // name
			R3rShaderVarStage::vertex, // stage
			R3rShaderVarType::attribute, // type
			R3rShaderVarTypeId::vec3, // type_id
			get_a_position_location(), // binding
		},

		{
			"a_color", // name
			R3rShaderVarStage::vertex, // stage
			R3rShaderVarType::attribute, // type
			R3rShaderVarTypeId::vec4, // type_id
			get_a_color_location(), // binding
		},

		{
			"a_tx_coords", // name
			R3rShaderVarStage::vertex, // stage
			R3rShaderVarType::attribute, // type
			R3rShaderVarTypeId::vec2, // type_id
			get_a_tx_coords_location(), // binding
		},

		{
			"u_model_mat", // name
			R3rShaderVarStage::vertex, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::mat4, // type_id
			0, // binding
		},

		{
			"u_view_mat", // name
			R3rShaderVarStage::vertex, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::mat4, // type_id
			0, // binding
		},

		{
			"u_projection_mat", // name
			R3rShaderVarStage::vertex, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::mat4, // type_id
			0, // binding
		},

		{
			"u_sampler", // name
			R3rShaderVarStage::fragment, // stage
			R3rShaderVarType::sampler, // type
			R3rShaderVarTypeId::sampler2d, // type_id
			1, // binding
		},

		{
			"u_shading_mode", // name
			R3rShaderVarStage::fragment, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::int32, // type_id
			2, // binding
		},

		{
			"u_shade_max", // name
			R3rShaderVarStage::fragment, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::float32, // type_id
			2, // binding
		},

		{
			"u_normal_shade", // name
			R3rShaderVarStage::fragment, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::float32, // type_id
			2, // binding
		},

		{
			"u_height_numerator", // name
			R3rShaderVarStage::fragment, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::float32, // type_id
			2, // binding
		},

		{
			"u_extra_lighting", // name
			R3rShaderVarStage::fragment, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::float32, // type_id
			2, // binding
		},

		{
			"u_view_direction", // name
			R3rShaderVarStage::fragment, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::vec2, // type_id
			2, // binding
		},

		{
			"u_view_position", // name
			R3rShaderVarStage::fragment, // stage
			R3rShaderVarType::uniform, // type
			R3rShaderVarTypeId::vec2, // type_id
			2, // binding
		},
	};

	return Span<const R3rShaderVarInfo>{result};
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

const R3rShaderSource& HwShaderRegistry::get_fragment_vk()
{
#ifndef BSTONE_VULKAN_COMPILE_SHADERS
// WARNING This file was generated by bstone_bin2c

const int byte_count = 3232;

alignas(std::uint32_t) static const unsigned char bytes[byte_count] =
{
	3,2,35,7,0,0,1,0,11,0,8,0,158,0,0,0,0,0,0,0,17,0,2,0,1,0,0,0,11,0,6,0,1,0,0,0,
	71,76,83,76,46,115,116,100,46,52,53,48,0,0,0,0,14,0,3,0,0,0,0,0,1,0,0,0,15,0,9,
	0,4,0,0,0,4,0,0,0,109,97,105,110,0,0,0,0,45,0,0,0,133,0,0,0,135,0,0,0,142,0,0,0,
	16,0,3,0,4,0,0,0,7,0,0,0,71,0,3,0,19,0,0,0,2,0,0,0,72,0,5,0,19,0,0,0,0,0,0,0,35,
	0,0,0,0,0,0,0,72,0,5,0,19,0,0,0,1,0,0,0,35,0,0,0,4,0,0,0,72,0,5,0,19,0,0,0,2,0,
	0,0,35,0,0,0,8,0,0,0,72,0,5,0,19,0,0,0,3,0,0,0,35,0,0,0,12,0,0,0,72,0,5,0,19,0,
	0,0,4,0,0,0,35,0,0,0,16,0,0,0,72,0,5,0,19,0,0,0,5,0,0,0,35,0,0,0,24,0,0,0,72,0,
	5,0,19,0,0,0,6,0,0,0,35,0,0,0,32,0,0,0,71,0,4,0,21,0,0,0,33,0,0,0,2,0,0,0,71,0,
	4,0,21,0,0,0,34,0,0,0,0,0,0,0,71,0,4,0,45,0,0,0,30,0,0,0,0,0,0,0,71,0,4,0,133,0,
	0,0,30,0,0,0,0,0,0,0,71,0,4,0,135,0,0,0,30,0,0,0,1,0,0,0,71,0,4,0,140,0,0,0,33,
	0,0,0,1,0,0,0,71,0,4,0,140,0,0,0,34,0,0,0,0,0,0,0,71,0,4,0,142,0,0,0,30,0,0,0,2,
	0,0,0,19,0,2,0,2,0,0,0,33,0,3,0,3,0,0,0,2,0,0,0,22,0,3,0,6,0,0,0,32,0,0,0,33,0,
	3,0,7,0,0,0,6,0,0,0,43,0,4,0,6,0,0,0,14,0,0,0,0,0,0,0,21,0,4,0,17,0,0,0,32,0,0,
	0,1,0,0,0,23,0,4,0,18,0,0,0,6,0,0,0,2,0,0,0,30,0,9,0,19,0,0,0,17,0,0,0,6,0,0,0,
	6,0,0,0,6,0,0,0,6,0,0,0,18,0,0,0,18,0,0,0,32,0,4,0,20,0,0,0,2,0,0,0,19,0,0,0,59,
	0,4,0,20,0,0,0,21,0,0,0,2,0,0,0,43,0,4,0,17,0,0,0,22,0,0,0,1,0,0,0,32,0,4,0,23,
	0,0,0,2,0,0,0,6,0,0,0,20,0,2,0,26,0,0,0,32,0,4,0,31,0,0,0,7,0,0,0,6,0,0,0,43,0,
	4,0,17,0,0,0,33,0,0,0,5,0,0,0,21,0,4,0,34,0,0,0,32,0,0,0,0,0,0,0,43,0,4,0,34,0,
	0,0,35,0,0,0,0,0,0,0,43,0,4,0,34,0,0,0,39,0,0,0,1,0,0,0,32,0,4,0,44,0,0,0,1,0,0,
	0,18,0,0,0,59,0,4,0,44,0,0,0,45,0,0,0,1,0,0,0,32,0,4,0,46,0,0,0,1,0,0,0,6,0,0,0,
	43,0,4,0,17,0,0,0,49,0,0,0,6,0,0,0,43,0,4,0,6,0,0,0,71,0,0,0,172,197,39,55,43,0,
	4,0,6,0,0,0,74,0,0,0,0,0,128,67,43,0,4,0,17,0,0,0,75,0,0,0,3,0,0,0,43,0,4,0,6,0,
	0,0,81,0,0,0,0,0,0,65,43,0,4,0,6,0,0,0,88,0,0,0,0,0,124,66,43,0,4,0,17,0,0,0,91,
	0,0,0,2,0,0,0,43,0,4,0,17,0,0,0,96,0,0,0,4,0,0,0,43,0,4,0,6,0,0,0,104,0,0,0,0,0,
	130,66,43,0,4,0,17,0,0,0,109,0,0,0,0,0,0,0,32,0,4,0,110,0,0,0,2,0,0,0,17,0,0,0,
	23,0,4,0,131,0,0,0,6,0,0,0,4,0,0,0,32,0,4,0,132,0,0,0,3,0,0,0,131,0,0,0,59,0,4,
	0,132,0,0,0,133,0,0,0,3,0,0,0,32,0,4,0,134,0,0,0,1,0,0,0,131,0,0,0,59,0,4,0,134,
	0,0,0,135,0,0,0,1,0,0,0,25,0,9,0,137,0,0,0,6,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,1,0,0,0,0,0,0,0,27,0,3,0,138,0,0,0,137,0,0,0,32,0,4,0,139,0,0,0,0,0,0,0,138,
	0,0,0,59,0,4,0,139,0,0,0,140,0,0,0,0,0,0,0,59,0,4,0,44,0,0,0,142,0,0,0,1,0,0,0,
	43,0,4,0,34,0,0,0,147,0,0,0,3,0,0,0,32,0,4,0,148,0,0,0,3,0,0,0,6,0,0,0,43,0,4,0,
	6,0,0,0,151,0,0,0,0,0,128,63,54,0,5,0,2,0,0,0,4,0,0,0,0,0,0,0,3,0,0,0,248,0,2,0,
	5,0,0,0,59,0,4,0,31,0,0,0,129,0,0,0,7,0,0,0,59,0,4,0,31,0,0,0,146,0,0,0,7,0,0,0,
	57,0,4,0,6,0,0,0,130,0,0,0,12,0,0,0,62,0,3,0,129,0,0,0,130,0,0,0,61,0,4,0,131,0,
	0,0,136,0,0,0,135,0,0,0,61,0,4,0,138,0,0,0,141,0,0,0,140,0,0,0,61,0,4,0,18,0,0,
	0,143,0,0,0,142,0,0,0,87,0,5,0,131,0,0,0,144,0,0,0,141,0,0,0,143,0,0,0,133,0,5,
	0,131,0,0,0,145,0,0,0,136,0,0,0,144,0,0,0,62,0,3,0,133,0,0,0,145,0,0,0,65,0,5,0,
	148,0,0,0,149,0,0,0,133,0,0,0,147,0,0,0,61,0,4,0,6,0,0,0,150,0,0,0,149,0,0,0,62,
	0,3,0,146,0,0,0,150,0,0,0,61,0,4,0,6,0,0,0,152,0,0,0,129,0,0,0,131,0,5,0,6,0,0,
	0,153,0,0,0,151,0,0,0,152,0,0,0,61,0,4,0,131,0,0,0,154,0,0,0,133,0,0,0,142,0,5,
	0,131,0,0,0,155,0,0,0,154,0,0,0,153,0,0,0,62,0,3,0,133,0,0,0,155,0,0,0,61,0,4,0,
	6,0,0,0,156,0,0,0,146,0,0,0,65,0,5,0,148,0,0,0,157,0,0,0,133,0,0,0,147,0,0,0,62,
	0,3,0,157,0,0,0,156,0,0,0,253,0,1,0,56,0,1,0,54,0,5,0,6,0,0,0,8,0,0,0,0,0,0,0,7,
	0,0,0,248,0,2,0,9,0,0,0,254,0,2,0,14,0,0,0,56,0,1,0,54,0,5,0,6,0,0,0,10,0,0,0,0,
	0,0,0,7,0,0,0,248,0,2,0,11,0,0,0,59,0,4,0,31,0,0,0,32,0,0,0,7,0,0,0,59,0,4,0,31,
	0,0,0,38,0,0,0,7,0,0,0,59,0,4,0,31,0,0,0,43,0,0,0,7,0,0,0,59,0,4,0,31,0,0,0,53,
	0,0,0,7,0,0,0,59,0,4,0,31,0,0,0,57,0,0,0,7,0,0,0,59,0,4,0,31,0,0,0,63,0,0,0,7,0,
	0,0,59,0,4,0,31,0,0,0,67,0,0,0,7,0,0,0,59,0,4,0,31,0,0,0,73,0,0,0,7,0,0,0,59,0,
	4,0,31,0,0,0,85,0,0,0,7,0,0,0,59,0,4,0,31,0,0,0,102,0,0,0,7,0,0,0,65,0,5,0,23,0,
	0,0,24,0,0,0,21,0,0,0,22,0,0,0,61,0,4,0,6,0,0,0,25,0,0,0,24,0,0,0,188,0,5,0,26,
	0,0,0,27,0,0,0,25,0,0,0,14,0,0,0,247,0,3,0,29,0,0,0,0,0,0,0,250,0,4,0,27,0,0,0,
	28,0,0,0,29,0,0,0,248,0,2,0,28,0,0,0,254,0,2,0,14,0,0,0,248,0,2,0,29,0,0,0,65,0,
	6,0,23,0,0,0,36,0,0,0,21,0,0,0,33,0,0,0,35,0,0,0,61,0,4,0,6,0,0,0,37,0,0,0,36,0,
	0,0,62,0,3,0,32,0,0,0,37,0,0,0,65,0,6,0,23,0,0,0,40,0,0,0,21,0,0,0,33,0,0,0,39,
	0,0,0,61,0,4,0,6,0,0,0,41,0,0,0,40,0,0,0,127,0,4,0,6,0,0,0,42,0,0,0,41,0,0,0,62,
	0,3,0,38,0,0,0,42,0,0,0,65,0,5,0,46,0,0,0,47,0,0,0,45,0,0,0,35,0,0,0,61,0,4,0,6,
	0,0,0,48,0,0,0,47,0,0,0,65,0,6,0,23,0,0,0,50,0,0,0,21,0,0,0,49,0,0,0,35,0,0,0,
	61,0,4,0,6,0,0,0,51,0,0,0,50,0,0,0,131,0,5,0,6,0,0,0,52,0,0,0,48,0,0,0,51,0,0,0,
	62,0,3,0,43,0,0,0,52,0,0,0,61,0,4,0,6,0,0,0,54,0,0,0,43,0,0,0,61,0,4,0,6,0,0,0,
	55,0,0,0,32,0,0,0,133,0,5,0,6,0,0,0,56,0,0,0,54,0,0,0,55,0,0,0,62,0,3,0,53,0,0,
	0,56,0,0,0,65,0,5,0,46,0,0,0,58,0,0,0,45,0,0,0,39,0,0,0,61,0,4,0,6,0,0,0,59,0,0,
	0,58,0,0,0,65,0,6,0,23,0,0,0,60,0,0,0,21,0,0,0,49,0,0,0,39,0,0,0,61,0,4,0,6,0,0,
	0,61,0,0,0,60,0,0,0,131,0,5,0,6,0,0,0,62,0,0,0,59,0,0,0,61,0,0,0,62,0,3,0,57,0,
	0,0,62,0,0,0,61,0,4,0,6,0,0,0,64,0,0,0,57,0,0,0,61,0,4,0,6,0,0,0,65,0,0,0,38,0,
	0,0,133,0,5,0,6,0,0,0,66,0,0,0,64,0,0,0,65,0,0,0,62,0,3,0,63,0,0,0,66,0,0,0,61,
	0,4,0,6,0,0,0,68,0,0,0,53,0,0,0,61,0,4,0,6,0,0,0,69,0,0,0,63,0,0,0,131,0,5,0,6,
	0,0,0,70,0,0,0,68,0,0,0,69,0,0,0,12,0,7,0,6,0,0,0,72,0,0,0,1,0,0,0,40,0,0,0,70,
	0,0,0,71,0,0,0,62,0,3,0,67,0,0,0,72,0,0,0,65,0,5,0,23,0,0,0,76,0,0,0,21,0,0,0,
	75,0,0,0,61,0,4,0,6,0,0,0,77,0,0,0,76,0,0,0,133,0,5,0,6,0,0,0,78,0,0,0,74,0,0,0,
	77,0,0,0,61,0,4,0,6,0,0,0,79,0,0,0,67,0,0,0,136,0,5,0,6,0,0,0,80,0,0,0,78,0,0,0,
	79,0,0,0,12,0,7,0,6,0,0,0,82,0,0,0,1,0,0,0,40,0,0,0,80,0,0,0,81,0,0,0,62,0,3,0,
	73,0,0,0,82,0,0,0,61,0,4,0,6,0,0,0,83,0,0,0,73,0,0,0,136,0,5,0,6,0,0,0,84,0,0,0,
	83,0,0,0,81,0,0,0,62,0,3,0,73,0,0,0,84,0,0,0,65,0,5,0,23,0,0,0,86,0,0,0,21,0,0,
	0,22,0,0,0,61,0,4,0,6,0,0,0,87,0,0,0,86,0,0,0,61,0,4,0,6,0,0,0,89,0,0,0,73,0,0,
	0,133,0,5,0,6,0,0,0,90,0,0,0,88,0,0,0,89,0,0,0,65,0,5,0,23,0,0,0,92,0,0,0,21,0,
	0,0,91,0,0,0,61,0,4,0,6,0,0,0,93,0,0,0,92,0,0,0,136,0,5,0,6,0,0,0,94,0,0,0,90,0,
	0,0,93,0,0,0,131,0,5,0,6,0,0,0,95,0,0,0,87,0,0,0,94,0,0,0,65,0,5,0,23,0,0,0,97,
	0,0,0,21,0,0,0,96,0,0,0,61,0,4,0,6,0,0,0,98,0,0,0,97,0,0,0,129,0,5,0,6,0,0,0,99,
	0,0,0,95,0,0,0,98,0,0,0,62,0,3,0,85,0,0,0,99,0,0,0,61,0,4,0,6,0,0,0,100,0,0,0,
	85,0,0,0,12,0,8,0,6,0,0,0,101,0,0,0,1,0,0,0,43,0,0,0,100,0,0,0,14,0,0,0,88,0,0,
	0,62,0,3,0,85,0,0,0,101,0,0,0,61,0,4,0,6,0,0,0,103,0,0,0,85,0,0,0,136,0,5,0,6,0,
	0,0,105,0,0,0,103,0,0,0,104,0,0,0,62,0,3,0,102,0,0,0,105,0,0,0,61,0,4,0,6,0,0,0,
	106,0,0,0,102,0,0,0,254,0,2,0,106,0,0,0,56,0,1,0,54,0,5,0,6,0,0,0,12,0,0,0,0,0,
	0,0,7,0,0,0,248,0,2,0,13,0,0,0,65,0,5,0,110,0,0,0,111,0,0,0,21,0,0,0,109,0,0,0,
	61,0,4,0,17,0,0,0,112,0,0,0,111,0,0,0,170,0,5,0,26,0,0,0,113,0,0,0,112,0,0,0,
	109,0,0,0,247,0,3,0,115,0,0,0,0,0,0,0,250,0,4,0,113,0,0,0,114,0,0,0,118,0,0,0,
	248,0,2,0,114,0,0,0,57,0,4,0,6,0,0,0,116,0,0,0,8,0,0,0,254,0,2,0,116,0,0,0,248,
	0,2,0,118,0,0,0,65,0,5,0,110,0,0,0,119,0,0,0,21,0,0,0,109,0,0,0,61,0,4,0,17,0,0,
	0,120,0,0,0,119,0,0,0,170,0,5,0,26,0,0,0,121,0,0,0,120,0,0,0,22,0,0,0,247,0,3,0,
	123,0,0,0,0,0,0,0,250,0,4,0,121,0,0,0,122,0,0,0,126,0,0,0,248,0,2,0,122,0,0,0,
	57,0,4,0,6,0,0,0,124,0,0,0,10,0,0,0,254,0,2,0,124,0,0,0,248,0,2,0,126,0,0,0,254,
	0,2,0,14,0,0,0,248,0,2,0,123,0,0,0,255,0,1,0,248,0,2,0,115,0,0,0,255,0,1,0,56,0,
	1,0,
};
#else
#include "default_frag.spv.h"
#endif

	static const R3rShaderSource result = {bytes, byte_count};
	return result;
}

const R3rShaderSource& HwShaderRegistry::get_vertex_vk()
{
#ifndef BSTONE_VULKAN_COMPILE_SHADERS
// WARNING This file was generated by bstone_bin2c

const int byte_count = 1676;

alignas(std::uint32_t) static const unsigned char bytes[byte_count] =
{
	3,2,35,7,0,0,1,0,11,0,8,0,66,0,0,0,0,0,0,0,17,0,2,0,1,0,0,0,11,0,6,0,1,0,0,0,71,
	76,83,76,46,115,116,100,46,52,53,48,0,0,0,0,14,0,3,0,0,0,0,0,1,0,0,0,15,0,12,0,
	0,0,0,0,4,0,0,0,109,97,105,110,0,0,0,0,21,0,0,0,31,0,0,0,35,0,0,0,37,0,0,0,39,0,
	0,0,41,0,0,0,55,0,0,0,71,0,3,0,11,0,0,0,2,0,0,0,72,0,4,0,11,0,0,0,0,0,0,0,5,0,0,
	0,72,0,5,0,11,0,0,0,0,0,0,0,7,0,0,0,16,0,0,0,72,0,5,0,11,0,0,0,0,0,0,0,35,0,0,0,
	0,0,0,0,72,0,4,0,11,0,0,0,1,0,0,0,5,0,0,0,72,0,5,0,11,0,0,0,1,0,0,0,7,0,0,0,16,
	0,0,0,72,0,5,0,11,0,0,0,1,0,0,0,35,0,0,0,64,0,0,0,72,0,4,0,11,0,0,0,2,0,0,0,5,0,
	0,0,72,0,5,0,11,0,0,0,2,0,0,0,7,0,0,0,16,0,0,0,72,0,5,0,11,0,0,0,2,0,0,0,35,0,0,
	0,128,0,0,0,71,0,4,0,13,0,0,0,33,0,0,0,0,0,0,0,71,0,4,0,13,0,0,0,34,0,0,0,0,0,0,
	0,71,0,4,0,21,0,0,0,30,0,0,0,0,0,0,0,71,0,4,0,31,0,0,0,30,0,0,0,0,0,0,0,71,0,4,
	0,35,0,0,0,30,0,0,0,1,0,0,0,71,0,4,0,37,0,0,0,30,0,0,0,1,0,0,0,71,0,4,0,39,0,0,
	0,30,0,0,0,2,0,0,0,71,0,4,0,41,0,0,0,30,0,0,0,2,0,0,0,71,0,3,0,53,0,0,0,2,0,0,0,
	72,0,5,0,53,0,0,0,0,0,0,0,11,0,0,0,0,0,0,0,72,0,5,0,53,0,0,0,1,0,0,0,11,0,0,0,1,
	0,0,0,72,0,5,0,53,0,0,0,2,0,0,0,11,0,0,0,3,0,0,0,72,0,5,0,53,0,0,0,3,0,0,0,11,0,
	0,0,4,0,0,0,19,0,2,0,2,0,0,0,33,0,3,0,3,0,0,0,2,0,0,0,22,0,3,0,6,0,0,0,32,0,0,0,
	23,0,4,0,7,0,0,0,6,0,0,0,4,0,0,0,32,0,4,0,8,0,0,0,7,0,0,0,7,0,0,0,24,0,4,0,10,0,
	0,0,7,0,0,0,4,0,0,0,30,0,5,0,11,0,0,0,10,0,0,0,10,0,0,0,10,0,0,0,32,0,4,0,12,0,
	0,0,2,0,0,0,11,0,0,0,59,0,4,0,12,0,0,0,13,0,0,0,2,0,0,0,21,0,4,0,14,0,0,0,32,0,
	0,0,1,0,0,0,43,0,4,0,14,0,0,0,15,0,0,0,0,0,0,0,32,0,4,0,16,0,0,0,2,0,0,0,10,0,0,
	0,23,0,4,0,19,0,0,0,6,0,0,0,3,0,0,0,32,0,4,0,20,0,0,0,1,0,0,0,19,0,0,0,59,0,4,0,
	20,0,0,0,21,0,0,0,1,0,0,0,43,0,4,0,6,0,0,0,23,0,0,0,0,0,128,63,23,0,4,0,29,0,0,
	0,6,0,0,0,2,0,0,0,32,0,4,0,30,0,0,0,3,0,0,0,29,0,0,0,59,0,4,0,30,0,0,0,31,0,0,0,
	3,0,0,0,32,0,4,0,34,0,0,0,3,0,0,0,7,0,0,0,59,0,4,0,34,0,0,0,35,0,0,0,3,0,0,0,32,
	0,4,0,36,0,0,0,1,0,0,0,7,0,0,0,59,0,4,0,36,0,0,0,37,0,0,0,1,0,0,0,59,0,4,0,30,0,
	0,0,39,0,0,0,3,0,0,0,32,0,4,0,40,0,0,0,1,0,0,0,29,0,0,0,59,0,4,0,40,0,0,0,41,0,
	0,0,1,0,0,0,21,0,4,0,42,0,0,0,32,0,0,0,0,0,0,0,43,0,4,0,42,0,0,0,43,0,0,0,0,0,0,
	0,32,0,4,0,44,0,0,0,1,0,0,0,6,0,0,0,43,0,4,0,42,0,0,0,47,0,0,0,1,0,0,0,28,0,4,0,
	52,0,0,0,6,0,0,0,47,0,0,0,30,0,6,0,53,0,0,0,7,0,0,0,6,0,0,0,52,0,0,0,52,0,0,0,
	32,0,4,0,54,0,0,0,3,0,0,0,53,0,0,0,59,0,4,0,54,0,0,0,55,0,0,0,3,0,0,0,43,0,4,0,
	14,0,0,0,56,0,0,0,2,0,0,0,43,0,4,0,14,0,0,0,59,0,0,0,1,0,0,0,54,0,5,0,2,0,0,0,4,
	0,0,0,0,0,0,0,3,0,0,0,248,0,2,0,5,0,0,0,59,0,4,0,8,0,0,0,9,0,0,0,7,0,0,0,65,0,5,
	0,16,0,0,0,17,0,0,0,13,0,0,0,15,0,0,0,61,0,4,0,10,0,0,0,18,0,0,0,17,0,0,0,61,0,
	4,0,19,0,0,0,22,0,0,0,21,0,0,0,81,0,5,0,6,0,0,0,24,0,0,0,22,0,0,0,0,0,0,0,81,0,
	5,0,6,0,0,0,25,0,0,0,22,0,0,0,1,0,0,0,81,0,5,0,6,0,0,0,26,0,0,0,22,0,0,0,2,0,0,
	0,80,0,7,0,7,0,0,0,27,0,0,0,24,0,0,0,25,0,0,0,26,0,0,0,23,0,0,0,145,0,5,0,7,0,0,
	0,28,0,0,0,18,0,0,0,27,0,0,0,62,0,3,0,9,0,0,0,28,0,0,0,61,0,4,0,7,0,0,0,32,0,0,
	0,9,0,0,0,79,0,7,0,29,0,0,0,33,0,0,0,32,0,0,0,32,0,0,0,0,0,0,0,1,0,0,0,62,0,3,0,
	31,0,0,0,33,0,0,0,61,0,4,0,7,0,0,0,38,0,0,0,37,0,0,0,62,0,3,0,35,0,0,0,38,0,0,0,
	65,0,5,0,44,0,0,0,45,0,0,0,41,0,0,0,43,0,0,0,61,0,4,0,6,0,0,0,46,0,0,0,45,0,0,0,
	65,0,5,0,44,0,0,0,48,0,0,0,41,0,0,0,47,0,0,0,61,0,4,0,6,0,0,0,49,0,0,0,48,0,0,0,
	131,0,5,0,6,0,0,0,50,0,0,0,23,0,0,0,49,0,0,0,80,0,5,0,29,0,0,0,51,0,0,0,46,0,0,
	0,50,0,0,0,62,0,3,0,39,0,0,0,51,0,0,0,65,0,5,0,16,0,0,0,57,0,0,0,13,0,0,0,56,0,
	0,0,61,0,4,0,10,0,0,0,58,0,0,0,57,0,0,0,65,0,5,0,16,0,0,0,60,0,0,0,13,0,0,0,59,
	0,0,0,61,0,4,0,10,0,0,0,61,0,0,0,60,0,0,0,146,0,5,0,10,0,0,0,62,0,0,0,58,0,0,0,
	61,0,0,0,61,0,4,0,7,0,0,0,63,0,0,0,9,0,0,0,145,0,5,0,7,0,0,0,64,0,0,0,62,0,0,0,
	63,0,0,0,65,0,5,0,34,0,0,0,65,0,0,0,55,0,0,0,15,0,0,0,62,0,3,0,65,0,0,0,64,0,0,
	0,253,0,1,0,56,0,1,0,
};
#else
#include "default_vert.spv.h"
#endif

	static const R3rShaderSource result = {bytes, byte_count};
	return result;
}

} // namespace bstone
