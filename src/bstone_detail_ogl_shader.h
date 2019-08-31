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


#include "bstone_ogl_api.h"
#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglShaderManager;
using OglShaderManagerPtr = OglShaderManager*;

class OglShaderStage;
using OglShaderStagePtr = OglShaderStage*;


// ==========================================================================
// OglShader
//

class OglShader :
	public RendererShader
{
protected:
	OglShader();


public:
	~OglShader() override;


	virtual GLuint get_ogl_name() const noexcept = 0;

	virtual void attach_to_shader_stage(
		const OglShaderStagePtr shader_stage) = 0;
}; // OglShader

using OglShaderPtr = OglShader*;
using OglShaderUPtr = std::unique_ptr<OglShader>;

//
// OglShader
// ==========================================================================


// ==========================================================================
// OglShaderFactory
//

struct OglShaderFactory final
{
	static OglShaderUPtr create(
		const OglShaderManagerPtr ogl_shader_manager,
		const RendererShaderCreateParam& param);
}; // OglShaderFactory

//
// OglShaderFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OLG_SHADER_INCLUDED
