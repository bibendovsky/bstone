/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
