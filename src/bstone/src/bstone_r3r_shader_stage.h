/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Shader Stage

#ifndef BSTONE_R3R_SHADER_STAGE_INCLUDED
#define BSTONE_R3R_SHADER_STAGE_INCLUDED

#include <memory>
#include <span>
#include "bstone_r3r_shader.h"
#include "bstone_r3r_shader_var.h"

namespace bstone {

struct R3rShaderStageInputBinding
{
	int index;
	const char* name;
};

using R3rShaderStageInputBindings = std::span<const R3rShaderStageInputBinding>;

// ==========================================================================

using R3rShaderStageShaderVarInfos = std::span<const R3rShaderVarInfo>;

// ==========================================================================

struct R3rShaderStageInitParam
{
	R3rShader* fragment_shader;
	R3rShader* vertex_shader;
	R3rShaderStageInputBindings input_bindings;
	R3rShaderStageShaderVarInfos shader_var_infos;
};

// ==========================================================================

class R3rShaderStage
{
public:
	R3rShaderStage() {};
	virtual ~R3rShaderStage() {};

	R3rShaderVar* find_var(const char* name);
	R3rShaderVar* find_int32_var(const char* name);
	R3rShaderVar* find_float32_var(const char* name);
	R3rShaderVar* find_vec2_var(const char* name);
	R3rShaderVar* find_vec3_var(const char* name);
	R3rShaderVar* find_vec4_var(const char* name);
	R3rShaderVar* find_mat4_var(const char* name);
	R3rShaderVar* find_r2_sampler_var(const char* name);

private:
	virtual R3rShaderVar* do_find_var(const char* name) = 0;
	virtual R3rShaderVar* do_find_int32_var(const char* name) = 0;
	virtual R3rShaderVar* do_find_float32_var(const char* name) = 0;
	virtual R3rShaderVar* do_find_vec2_var(const char* name) = 0;
	virtual R3rShaderVar* do_find_vec3_var(const char* name) = 0;
	virtual R3rShaderVar* do_find_vec4_var(const char* name) = 0;
	virtual R3rShaderVar* do_find_mat4_var(const char* name) = 0;
	virtual R3rShaderVar* do_find_r2_sampler_var(const char* name) = 0;
};

// ==========================================================================

using R3rShaderStageUPtr = std::unique_ptr<R3rShaderStage>;

} // namespace bstone

#endif // BSTONE_R3R_SHADER_STAGE_INCLUDED
