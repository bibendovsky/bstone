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
// OpenGL shader (implementation).
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_OLG_RENDERER_SHADER_IMPL_INCLUDED
#define BSTONE_DETAIL_OLG_RENDERER_SHADER_IMPL_INCLUDED


#include "bstone_renderer.h"
#include "bstone_ogl_raii.h"


namespace bstone
{
namespace detail
{


class OglRendererShaderStageImpl;
using OglRendererShaderStageImplPtr = OglRendererShaderStageImpl*;


class OglRendererShaderImpl final :
	public RendererShader
{
public:
	OglRendererShaderImpl();

	OglRendererShaderImpl(
		const OglRendererShaderImpl& rhs) = delete;

	OglRendererShaderImpl(
		OglRendererShaderImpl&& rhs) = default;

	~OglRendererShaderImpl() override;


	bool is_initialized() const override;

	const std::string& get_error_message() const override;

	Kind get_kind() const override;


	void initialize(
		const RendererShader::CreateParam& param);

	GLuint get_ogl_name() const;

	void attach_to_shader_stage(
		const OglRendererShaderStageImplPtr shader_stage);


private:
	std::string error_message_;

	Kind kind_;
	OglShaderRaii ogl_name_raii_;
	OglRendererShaderStageImplPtr shader_stage_;


	GLenum get_ogl_kind(
		const Kind kind);

	bool validate_param(
		const RendererShader::CreateParam& param);
}; // OglRendererShaderImpl

using OglRendererShaderImplPtr = OglRendererShaderImpl*;
using OglRendererShaderImplUPtr = std::unique_ptr<OglRendererShaderImpl>;


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OLG_RENDERER_SHADER_IMPL_INCLUDED
