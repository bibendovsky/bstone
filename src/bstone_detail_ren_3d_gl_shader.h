/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


#ifndef BSTONE_DETAIL_REN_3D_GL_SHADER_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_SHADER_INCLUDED


#include "bstone_ren_3d_shader.h"

#include "bstone_detail_ren_3d_gl_api.h"


namespace bstone
{
namespace detail
{


class Ren3dGlShaderMgr;
using Ren3dGlShaderMgrPtr = Ren3dGlShaderMgr*;

class Ren3dGlShaderStage;
using Ren3dGlShaderStagePtr = Ren3dGlShaderStage*;


// ==========================================================================
// Ren3dGlShader
//

class Ren3dGlShader :
	public Ren3dShader
{
protected:
	Ren3dGlShader() = default;


public:
	~Ren3dGlShader() override = default;


	virtual GLuint get_gl_name() const noexcept = 0;

	virtual void attach_to_shader_stage(
		const Ren3dGlShaderStagePtr shader_stage) = 0;
}; // Ren3dGlShader

using Ren3dGlShaderPtr = Ren3dGlShader*;
using Ren3dGlShaderUPtr = std::unique_ptr<Ren3dGlShader>;

//
// Ren3dGlShader
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderFactory
//

struct Ren3dGlShaderFactory
{
	static Ren3dGlShaderUPtr create(
		const Ren3dCreateShaderParam& param);
}; // Ren3dGlShaderFactory

//
// Ren3dGlShaderFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_SHADER_INCLUDED
