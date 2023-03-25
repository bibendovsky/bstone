/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Limits

#if !defined(BSTONE_R3R_LIMITS_INCLUDED)
#define BSTONE_R3R_LIMITS_INCLUDED

namespace bstone {

struct R3rLimits
{
	static constexpr int min_anisotropy_off = 1;
	static constexpr int min_anisotropy_on = 2;
	static constexpr int max_anisotropy = 16;

	static constexpr int min_aa_off = 1;
	static constexpr int min_aa_on = 2;
	static constexpr int max_aa = 32;

	static constexpr int max_mipmap_count = 31;

	static constexpr int max_buffers = 32;

	static constexpr int max_textures = 4096;
	static constexpr int max_samplers = 8;

	static constexpr int max_shaders = 2;
	static constexpr int max_shader_stages = 1;
	static constexpr int max_shader_vars = 16;

	static constexpr int max_vertex_inputs = 16;
};

} // namespace bstone

#endif // BSTONE_R3R_LIMITS_INCLUDED
