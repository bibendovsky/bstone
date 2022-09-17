/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
