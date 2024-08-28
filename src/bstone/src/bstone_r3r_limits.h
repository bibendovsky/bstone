/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Limits

#ifndef BSTONE_R3R_LIMITS_INCLUDED
#define BSTONE_R3R_LIMITS_INCLUDED

namespace bstone {

// TODO Use data member in C++17.
struct R3rLimits
{
	static constexpr int min_anisotropy_off() { return 1; }
	static constexpr int min_anisotropy_on() { return 2; }
	static constexpr int max_anisotropy() { return 16; }

	static constexpr int min_aa_off() { return 1; }
	static constexpr int min_aa_on() { return 2; }
	static constexpr int max_aa() { return 32; }

	static constexpr int max_mipmap_count() { return 31; }

	static constexpr int max_buffers() { return 32; }

	static constexpr int max_textures() { return 4096; }
	static constexpr int max_samplers() { return 8; }

	static constexpr int max_shaders() { return 2; }
	static constexpr int max_shader_stages() { return 1; }
	static constexpr int max_shader_vars() { return 16; }

	static constexpr int max_vertex_inputs() { return 16; }
};

} // namespace bstone

#endif // BSTONE_R3R_LIMITS_INCLUDED
