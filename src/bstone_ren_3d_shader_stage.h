/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// 3D renderer's shader stage.
//


#ifndef BSTONE_REN_3D_SHADER_STAGE_INCLUDED
#define BSTONE_REN_3D_SHADER_STAGE_INCLUDED


#include <memory>
#include <string>
#include <vector>

#include "bstone_ren_3d_shader.h"
#include "bstone_ren_3d_shader_stage.h"
#include "bstone_ren_3d_shader_var.h"


namespace bstone
{


// ==========================================================================
// Ren3dShaderStage
//

struct Ren3dShaderStageInputBinding
{
	int index_;
	std::string name_;
}; // Ren3dShaderStageInputBinding

using Ren3dShaderStageInputBindings = std::vector<Ren3dShaderStageInputBinding>;


struct Ren3dCreateShaderStageParam
{
	Ren3dShaderPtr fragment_shader_;
	Ren3dShaderPtr vertex_shader_;
	Ren3dShaderStageInputBindings input_bindings_;
}; // Ren3dCreateShaderStageParam


class Ren3dShaderStage
{
public:
	Ren3dShaderStage() noexcept = default;

	virtual ~Ren3dShaderStage() = default;


	virtual Ren3dShaderVarPtr find_var(
		const std::string& name) noexcept = 0;

	virtual Ren3dShaderInt32VarPtr find_int32_var(
		const std::string& name) noexcept = 0;

	virtual Ren3dShaderFloat32VarPtr find_float32_var(
		const std::string& name) noexcept = 0;

	virtual Ren3dShaderVec2VarPtr find_vec2_var(
		const std::string& name) noexcept = 0;

	virtual Ren3dShaderVec4VarPtr find_vec4_var(
		const std::string& name) noexcept = 0;

	virtual Ren3dShaderMat4VarPtr find_mat4_var(
		const std::string& name) noexcept = 0;

	virtual Ren3dShaderSampler2dVarPtr find_sampler_2d_var(
		const std::string& name) noexcept = 0;
}; // Ren3dShaderStage

using Ren3dShaderStagePtr = Ren3dShaderStage*;
using Ren3dShaderStageUPtr = std::unique_ptr<Ren3dShaderStage>;

//
// Ren3dShaderStage
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_SHADER_STAGE_INCLUDED
