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
// OpenGL shader stage (implementation).
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_OLG_SHADER_STAGE_INCLUDED
#define BSTONE_DETAIL_OLG_SHADER_STAGE_INCLUDED


#include "bstone_detail_ogl_shader.h"
#include "bstone_detail_ogl_shader_variable.h"


namespace bstone
{
namespace detail
{


class OglShaderStage;
using OglShaderStagePtr = OglShaderStage*;


class OglShaderStage final :
	public RendererShaderStage
{
public:
	OglShaderStage();

	OglShaderStage(
		const OglShaderStage& rhs) = delete;

	OglShaderStage(
		OglShaderStage&& rhs) = default;

	~OglShaderStage() override;


	bool is_initialized() const override;

	const std::string& get_error_message() const override;

	void set_current() override;

	RendererShaderVariablePtr find_variable(
		const std::string& name) override;

	RendererShaderVariableInt32Ptr find_variable_int32(
		const std::string& name) override;

	RendererShaderVariableFloat32Ptr find_variable_float32(
		const std::string& name) override;

	RendererShaderVariableVec2Ptr find_variable_vec2(
		const std::string& name) override;

	RendererShaderVariableVec4Ptr find_variable_vec4(
		const std::string& name) override;

	RendererShaderVariableMat4Ptr find_variable_mat4(
		const std::string& name) override;

	RendererShaderVariableSampler2dPtr find_variable_sampler_2d(
		const std::string& name) override;


	void initialize(
		OglShaderStagePtr* current_shader_stage_ptr,
		const RendererShaderStage::CreateParam& param);

	void detach_fragment_shader();

	void detach_vertex_shader();

	static void unset_current();


private:
	class Detail;


	using NameBuffer = std::vector<char>;
	using ShaderVariables = std::vector<OglShaderVariable>;


	bool is_initialized_;
	std::string error_message_;

	OglShaderStagePtr* current_shader_stage_ptr_;
	OglShaderPtr fragment_shader_;
	OglShaderPtr vertex_shader_;
	OglProgramHandle ogl_handle_;
	ShaderVariables shader_variables_;


	bool validate_shader(
		const RendererShader::Kind shader_kind,
		const RendererShaderPtr shader);

	bool validate_input_bindings(
		const InputBindings& input_bindings);

	bool validate_param(
		const RendererShaderStage::CreateParam& param);

	void set_input_bindings(
		const GLuint ogl_name,
		const InputBindings& input_bindings);

	int get_variable_count(
		const GLuint ogl_name);

	bool get_variables(
		const RendererShaderVariable::Kind kind,
		const GLuint ogl_name,
		ShaderVariables& shader_variables);

	bool check_input_bindings(
		const InputBindings& input_bindings,
		const ShaderVariables& variables);
}; // OglShaderStage

using OglShaderStagePtr = OglShaderStage*;
using OglShaderStageUPtr = std::unique_ptr<OglShaderStage>;


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OLG_SHADER_STAGE_INCLUDED
