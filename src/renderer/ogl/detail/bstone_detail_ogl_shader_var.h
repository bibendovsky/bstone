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
// OpenGL shader variable (implementation).
//
// !!! Internal usage only. !!!
//


#ifndef BSTONE_DETAIL_OLG_SHADER_VAR_INCLUDED
#define BSTONE_DETAIL_OLG_SHADER_VAR_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglShaderStage;
using OglShaderStagePtr = OglShaderStage*;


// ==========================================================================
// OglShaderVar
//

class OglShaderVar :
	public virtual RendererShaderVarInt32,
	public virtual RendererShaderVarFloat32,
	public virtual RendererShaderVarVec2,
	public virtual RendererShaderVarVec4,
	public virtual RendererShaderVarMat4
{
protected:
	OglShaderVar();


public:
	~OglShaderVar() override;


	static int get_unit_size(
		const RendererShaderVarTypeId type_id);
}; // OglShaderVar

using OglShaderVarPtr = OglShaderVar*;
using OglShaderVarUPtr = std::unique_ptr<OglShaderVar>;

//
// OglShaderVar
// ==========================================================================


// ==========================================================================
// OglShaderVarFactory
//

struct OglShaderVarFactory final
{
	struct CreateParam final
	{
		RendererShaderVarKind kind_;
		RendererShaderVarTypeId type_id_;
		int value_size_;
		int index_;
		std::string name_;
		int input_index_;
		int ogl_location_;
	}; // CreateParam

	static OglShaderVarUPtr create(
		const OglShaderStagePtr shader_stage,
		const CreateParam& param);
}; // OglShaderVarFactory

//
// OglShaderVarFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OLG_SHADER_VAR_INCLUDED
