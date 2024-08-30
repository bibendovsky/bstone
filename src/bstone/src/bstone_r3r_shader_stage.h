/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Shader Stage

#ifndef BSTONE_R3R_SHADER_STAGE_INCLUDED
#define BSTONE_R3R_SHADER_STAGE_INCLUDED

#include <memory>
#include "bstone_span.h"
#include "bstone_r3r_shader.h"
#include "bstone_r3r_shader_var.h"

namespace bstone {

struct R3rShaderStageInputBinding
{
	int index;
	const char* name;
};

using R3rShaderStageInputBindings = Span<const R3rShaderStageInputBinding>;

// ==========================================================================

struct R3rShaderStageInitParam
{
	R3rShader* fragment_shader;
	R3rShader* vertex_shader;
	R3rShaderStageInputBindings input_bindings;
};

// ==========================================================================

class R3rShaderStage
{
public:
	R3rShaderStage() noexcept;
	virtual ~R3rShaderStage();

	R3rShaderVar* find_var(const char* name) noexcept;
	R3rShaderInt32Var* find_int32_var(const char* name) noexcept;
	R3rShaderFloat32Var* find_float32_var(const char* name) noexcept;
	R3rShaderVec2Var* find_vec2_var(const char* name) noexcept;
	R3rShaderVec4Var* find_vec4_var(const char* name) noexcept;
	R3rShaderMat4Var* find_mat4_var(const char* name) noexcept;
	R3rShaderR2SamplerVar* find_r2_sampler_var(const char* name) noexcept;

private:
	virtual R3rShaderVar* do_find_var(const char* name) noexcept = 0;
	virtual R3rShaderInt32Var* do_find_int32_var(const char* name) noexcept = 0;
	virtual R3rShaderFloat32Var* do_find_float32_var(const char* name) noexcept = 0;
	virtual R3rShaderVec2Var* do_find_vec2_var(const char* name) noexcept = 0;
	virtual R3rShaderVec4Var* do_find_vec4_var(const char* name) noexcept = 0;
	virtual R3rShaderMat4Var* do_find_mat4_var(const char* name) noexcept = 0;
	virtual R3rShaderR2SamplerVar* do_find_r2_sampler_var(const char* name) noexcept = 0;
};

// ==========================================================================

using R3rShaderStageUPtr = std::unique_ptr<R3rShaderStage>;

} // namespace bstone

#endif // BSTONE_R3R_SHADER_STAGE_INCLUDED
