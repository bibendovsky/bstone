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
// OpenGL shader stage manager (implementation interface).
//


#ifndef BSTONE_DETAIL_REN_3D_GL_SHADER_STAGE_MGR_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_SHADER_STAGE_MGR_INCLUDED


#include <memory>

#include "bstone_ren_3d_shader_stage.h"


namespace bstone
{
namespace detail
{


class Ren3dGlContext;
using Ren3dGlContextPtr = Ren3dGlContext*;


// ==========================================================================
// Ren3dGlShaderStageMgr
//

class Ren3dGlShaderStageMgr
{
protected:
	Ren3dGlShaderStageMgr() = default;


public:
	virtual ~Ren3dGlShaderStageMgr() = default;


	virtual Ren3dGlContextPtr get_context() const noexcept = 0;


	virtual Ren3dShaderStageUPtr create(
		const Ren3dCreateShaderStageParam& param) = 0;


	virtual void set(
		const Ren3dShaderStagePtr shader_stage) = 0;
}; // Ren3dGlShaderStageMgr

using Ren3dGlShaderStageMgrPtr = Ren3dGlShaderStageMgr*;
using Ren3dGlShaderStageMgrUPtr = std::unique_ptr<Ren3dGlShaderStageMgr>;

//
// Ren3dGlShaderStageMgr
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderStageMgrFactory
//

struct Ren3dGlShaderStageMgrFactory
{
	static Ren3dGlShaderStageMgrUPtr create(
		const Ren3dGlContextPtr context);
}; // Ren3dGlShaderStageMgrFactory

//
// Ren3dGlShaderStageMgr
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_SHADER_STAGE_MGR_INCLUDED
