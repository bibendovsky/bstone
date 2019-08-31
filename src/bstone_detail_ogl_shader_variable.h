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


#ifndef BSTONE_DETAIL_OLG_SHADER_VARIABLE_INCLUDED
#define BSTONE_DETAIL_OLG_SHADER_VARIABLE_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglShaderStage;
using OglShaderStagePtr = OglShaderStage*;


// ==========================================================================
// OglShaderVariable
//

class OglShaderVariable :
	public virtual RendererShaderVariableInt32,
	public virtual RendererShaderVariableFloat32,
	public virtual RendererShaderVariableVec2,
	public virtual RendererShaderVariableVec4,
	public virtual RendererShaderVariableMat4
{
protected:
	OglShaderVariable();


public:
	~OglShaderVariable() override;


	static int get_unit_size(
		const RendererShaderVariableTypeId type_id);
}; // OglShaderVariable

using OglShaderVariablePtr = OglShaderVariable*;
using OglShaderVariableUPtr = std::unique_ptr<OglShaderVariable>;

//
// OglShaderVariable
// ==========================================================================


// ==========================================================================
// OglShaderVariableFactory
//

struct OglShaderVariableFactory final
{
	struct CreateParam final
	{
		RendererShaderVariableKind kind_;
		RendererShaderVariableTypeId type_id_;
		int value_size_;
		int index_;
		std::string name_;
		int input_index_;
		int ogl_location_;
	}; // CreateParam

	static OglShaderVariableUPtr create(
		const OglShaderStagePtr shader_stage,
		const CreateParam& param);
}; // OglShaderVariableFactory

//
// OglShaderVariableFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OLG_SHADER_VARIABLE_INCLUDED
