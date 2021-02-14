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
// OpenGL shader stage (implementation).
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_SHADER_STAGE_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_SHADER_STAGE_INCLUDED


#include "bstone_ren_3d_shader_stage.h"

#include "bstone_detail_ren_3d_gl_api.h"


namespace bstone
{
namespace detail
{


class Ren3dGlShaderStageMgr;
using Ren3dGlShaderStageMgrPtr = Ren3dGlShaderStageMgr*;


// ==========================================================================
// Ren3dGlShaderStage
//

class Ren3dGlShaderStage :
	public Ren3dShaderStage
{
protected:
	Ren3dGlShaderStage() = default;


public:
	~Ren3dGlShaderStage() override = default;


	virtual Ren3dGlShaderStageMgrPtr get_manager() const noexcept = 0;


	virtual void set() = 0;

	virtual void detach_fragment_shader() = 0;

	virtual void detach_vertex_shader() = 0;

	virtual GLuint get_gl_name() const noexcept = 0;
}; // Ren3dGlShaderStage

using Ren3dGlShaderStagePtr = Ren3dGlShaderStage*;
using Ren3dGlShaderStageUPtr = std::unique_ptr<Ren3dGlShaderStage>;

//
// Ren3dGlShaderStage
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderStageFactory
//

struct Ren3dGlShaderStageFactory
{
	static Ren3dGlShaderStageUPtr create(
		const Ren3dGlShaderStageMgrPtr shader_stage_manager,
		const Ren3dCreateShaderStageParam& param);
}; // Ren3dGlShaderStageFactory

//
// Ren3dGlShaderStageFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_SHADER_STAGE_INCLUDED
