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
// OpenGL shader object (implementation).
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_OLG_SHADER_INCLUDED
#define BSTONE_DETAIL_OLG_SHADER_INCLUDED


#include "bstone_renderer.h"
#include "bstone_ogl_handles.h"


namespace bstone
{
namespace detail
{


class OglShaderStage;
using OglShaderStagePtr = OglShaderStage*;


class OglShader final :
	public RendererShader
{
public:
	OglShader();

	OglShader(
		const OglShader& rhs) = delete;

	OglShader(
		OglShader&& rhs) = default;

	~OglShader() override;


	bool is_initialized() const override;

	const std::string& get_error_message() const override;

	Kind get_kind() const override;


	void initialize(
		const RendererShader::CreateParam& param);

	GLuint get_ogl_name() const;

	void attach_to_shader_stage(
		const OglShaderStagePtr shader_stage);


private:
	std::string error_message_;

	Kind kind_;
	OglShaderHandle ogl_handle_;
	OglShaderStagePtr shader_stage_;


	GLenum get_ogl_kind(
		const Kind kind);

	bool validate_param(
		const RendererShader::CreateParam& param);
}; // OglShader

using OglShaderPtr = OglShader*;
using OglShaderUPtr = std::unique_ptr<OglShader>;


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OLG_SHADER_INCLUDED
