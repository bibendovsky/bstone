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
// Notes:
// - See CalcHeight and ScalePost in 3d_draw.cpp for shading calculation.
//



#include "bstone_precompiled.h"
#include "bstone_renderer_shader_registry.h"


namespace bstone
{


int RendererShaderRegistry::get_a_position_location()
{
	return 0;
}

int RendererShaderRegistry::get_a_color_location()
{
	return 1;
}

int RendererShaderRegistry::get_a_tx_coords_location()
{
	return 2;
}

const std::string& RendererShaderRegistry::get_a_position_name()
{
	static const auto result = std::string{"a_position"};

	return result;
}

const std::string& RendererShaderRegistry::get_a_color_name()
{
	static const auto result = std::string{"a_color"};

	return result;
}

const std::string& RendererShaderRegistry::get_a_tx_coords_name()
{
	static const auto result = std::string{"a_tx_coords"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_model_mat_name()
{
	static const auto result = std::string{"u_model_mat"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_view_mat_name()
{
	static const auto result = std::string{"u_view_mat"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_projection_mat_name()
{
	static const auto result = std::string{"u_projection_mat"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_texture_mat_name()
{
	static const auto result = std::string{"u_texture_mat"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_sampler_name()
{
	static const auto result = std::string{"u_sampler"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_shading_mode_name()
{
	static const auto result = std::string{"u_shading_mode"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_shade_max_name()
{
	static const auto result = std::string{"u_shade_max"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_normal_shade_name()
{
	static const auto result = std::string{"u_normal_shade"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_height_numerator_name()
{
	static const auto result = std::string{"u_height_numerator"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_extra_lighting_name()
{
	static const auto result = std::string{"u_extra_lighting"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_view_direction_name()
{
	static const auto result = std::string{"u_view_direction"};

	return result;
}

const std::string& RendererShaderRegistry::get_u_view_position_name()
{
	static const auto result = std::string{"u_view_position"};

	return result;
}

const RendererShaderSource& RendererShaderRegistry::get_fragment(
	const RendererKind renderer_kind)
{
	switch (renderer_kind)
	{
		case RendererKind::ogl_2_x:
			return get_fragment_ogl_2_x();

		default:
			return get_empty();
	}
}

const RendererShaderSource& RendererShaderRegistry::get_vertex(
	const RendererKind renderer_kind)
{
	switch (renderer_kind)
	{
		case RendererKind::ogl_2_x:
			return get_vertex_ogl_2_x();

		default:
			return get_empty();
	}
}

const RendererShaderSource& RendererShaderRegistry::get_empty()
{
	static const auto result = RendererShaderSource{};

	return result;
}

const RendererShaderSource& RendererShaderRegistry::get_fragment_ogl_2_x()
{
	static const auto source = std::string
	{
R"FRAGMENT_SHADER(
#version 110


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
)FRAGMENT_SHADER"
	};

	static const auto result = RendererShaderSource
	{
		source.c_str(),
		static_cast<int>(source.size()),
	};

	return result;
}

const RendererShaderSource& RendererShaderRegistry::get_vertex_ogl_2_x()
{
	static const auto source = std::string
	{
R"VERTEX_SHADER(
#version 110


attribute vec3 a_position;
attribute vec4 a_color;
attribute vec2 a_tx_coords;


uniform mat4 u_model_mat;
uniform mat4 u_view_mat;
uniform mat4 u_projection_mat;
uniform mat4 u_texture_mat;


varying vec2 o_position;
varying vec4 o_color;
varying vec2 o_tx_coords;


void main()
{
	vec4 position = u_model_mat * vec4(a_position, 1.0);

	o_position = position.xy;
	o_color = a_color;
	o_tx_coords = (vec4(a_tx_coords, 0.0, 1.0) * u_texture_mat).xy;

	gl_Position = u_projection_mat * u_view_mat * position;
}
)VERTEX_SHADER"
	};

	static const auto result = RendererShaderSource
	{
		source.c_str(),
		static_cast<int>(source.size()),
	};

	return result;
}


} // bstone
