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
// OpenGL shader variable (implementation).
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_SHADER_VAR_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_SHADER_VAR_INCLUDED


#include "bstone_ren_3d_shader_var.h"


namespace bstone
{
namespace detail
{


class Ren3dGlShaderStage;
using Ren3dGlShaderStagePtr = Ren3dGlShaderStage*;


// ==========================================================================
// Ren3dGlShaderVar
//

struct Ren3dGlShaderVarCreateParam
{
	Ren3dShaderVarKind kind_;
	Ren3dShaderVarTypeId type_id_;
	int value_size_;
	int index_;
	std::string name_;
	int input_index_;
	int gl_location_;
}; // Ren3dGlShaderVarCreateParam


class Ren3dGlShaderVar :
	public virtual Ren3dShaderVarInt32,
	public virtual Ren3dShaderVarFloat32,
	public virtual Ren3dShaderVarVec2,
	public virtual Ren3dShaderVarVec4,
	public virtual Ren3dShaderVarMat4,
	public virtual Ren3dShaderVarSampler2d
{
protected:
	Ren3dGlShaderVar() = default;


public:
	~Ren3dGlShaderVar() override = default;


	static int get_unit_size(
		const Ren3dShaderVarTypeId type_id);
}; // Ren3dGlShaderVar

using Ren3dGlShaderVarUPtr = std::unique_ptr<Ren3dGlShaderVar>;

//
// Ren3dGlShaderVar
// ==========================================================================


// ==========================================================================
// Ren3dGlShaderVarFactory
//

struct Ren3dGlShaderVarFactory
{
	static Ren3dGlShaderVarUPtr create(
		const Ren3dGlShaderStagePtr shader_stage,
		const Ren3dGlShaderVarCreateParam& param);
}; // Ren3dGlShaderVarFactory

//
// Ren3dGlShaderVarFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_SHADER_VAR_INCLUDED
