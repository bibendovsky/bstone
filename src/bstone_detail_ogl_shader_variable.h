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


#include "bstone_ogl_api.h"
#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglShaderStage;
using OglShaderStagePtr = OglShaderStage*;


class OglShaderVariable :
	public virtual RendererShaderVariableInt32,
	public virtual RendererShaderVariableFloat32,
	public virtual RendererShaderVariableVec2,
	public virtual RendererShaderVariableVec4,
	public virtual RendererShaderVariableMat4
{
public:
	Kind kind_;
	TypeId type_id_;
	int value_size_;
	int index_;
	std::string name_;
	int input_index_;
	GLint ogl_location_;
	OglShaderStagePtr shader_stage_;


	OglShaderVariable();

	~OglShaderVariable() override;


	Kind get_kind() const override;

	TypeId get_type_id() const override;

	int get_index() const override;

	const std::string& get_name() const override;

	int get_input_index() const override;


	void set_value(
		const std::int32_t value) override;

	void set_value(
		const float value) override;

	void set_value(
		const glm::vec2& value) override;

	void set_value(
		const glm::vec4& value) override;

	void set_value(
		const glm::mat4& value) override;


	static int get_unit_size(
		const TypeId type_id);


private:
	void set_value(
		const TypeId type_id,
		const void* const value_data);

	void set_value(
		const void* const value_data);
}; // OglShaderVariable

using OglShaderVariablePtr = OglShaderVariable*;
using OglShaderVariableUPtr = std::unique_ptr<OglShaderVariable>;


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OLG_SHADER_VARIABLE_INCLUDED
