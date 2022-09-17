/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
	Ren3dShaderVarKind kind;
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
