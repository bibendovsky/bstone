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
// OpenGL shader stage (implementation).
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_OLG_SHADER_STAGE_INCLUDED
#define BSTONE_DETAIL_OLG_SHADER_STAGE_INCLUDED


#include "bstone_detail_ogl_shader.h"
#include "bstone_detail_ogl_shader_variable.h"


namespace bstone
{
namespace detail
{


class OglShaderStageManager;
using OglShaderStageManagerPtr = OglShaderStageManager*;


// ==========================================================================
// OglShaderStage
//

class OglShaderStage :
	public RendererShaderStage
{
protected:
	OglShaderStage();


public:
	~OglShaderStage() override;


	virtual void detach_fragment_shader() = 0;

	virtual void detach_vertex_shader() = 0;

	static void unset_current();
}; // OglShaderStage

using OglShaderStagePtr = OglShaderStage*;
using OglShaderStageUPtr = std::unique_ptr<OglShaderStage>;

//
// OglShaderStage
// ==========================================================================


// ==========================================================================
// OglShaderStageFactory
//

struct OglShaderStageFactory final
{
	static OglShaderStageUPtr create(
		const OglShaderStageManagerPtr ogl_shader_stage_manager,
		const RendererShaderStage::CreateParam& param);
}; //  OglShaderStageFactory

//
// OglShaderStageFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OLG_SHADER_STAGE_INCLUDED
