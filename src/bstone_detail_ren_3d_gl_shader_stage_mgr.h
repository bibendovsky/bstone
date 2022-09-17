/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
public:
	Ren3dGlShaderStageMgr() noexcept = default;

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
