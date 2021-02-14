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
// 3D renderer's shader variable.
//


#ifndef BSTONE_REN_3D_SHADER_VAR_INCLUDED
#define BSTONE_REN_3D_SHADER_VAR_INCLUDED


#include <memory>
#include <string>


namespace bstone
{


// ==========================================================================
// Ren3dShaderVar
//

enum class Ren3dShaderVarKind
{
	none,
	attribute,
	sampler,
	uniform,
}; // Ren3dShaderVarKind

enum class Ren3dShaderVarTypeId
{
	none,
	int32,
	float32,
	vec2,
	vec3,
	vec4,
	mat4,
	sampler2d,
}; // Ren3dShaderVarTypeId


class Ren3dShaderVar
{
protected:
	Ren3dShaderVar() = default;

	virtual ~Ren3dShaderVar() = default;


public:
	virtual Ren3dShaderVarKind get_kind() const noexcept = 0;

	virtual Ren3dShaderVarTypeId get_type_id() const noexcept = 0;

	virtual int get_index() const noexcept = 0;

	virtual const std::string& get_name() const noexcept = 0;

	virtual int get_input_index() const noexcept = 0;
}; // Ren3dShaderVar

using Ren3dShaderVarPtr = Ren3dShaderVar*;

//
// Ren3dShaderVar
// ==========================================================================


// ==========================================================================
// Ren3dShaderVarInt32
//

class Ren3dShaderVarInt32 :
	public virtual Ren3dShaderVar
{
protected:
	Ren3dShaderVarInt32() = default;

	virtual ~Ren3dShaderVarInt32() = default;


public:
	virtual void set_int32(
		const std::int32_t value) = 0;
}; // Ren3dShaderVarInt32

using Ren3dShaderInt32VarPtr = Ren3dShaderVarInt32*;

//
// Ren3dShaderVarInt32
// ==========================================================================


// ==========================================================================
// Ren3dShaderVarFloat32
//

class Ren3dShaderVarFloat32 :
	public virtual Ren3dShaderVar
{
protected:
	Ren3dShaderVarFloat32() = default;

	virtual ~Ren3dShaderVarFloat32() = default;


public:
	virtual void set_float32(
		const float value) = 0;
}; // Ren3dShaderVarFloat32

using Ren3dShaderFloat32VarPtr = Ren3dShaderVarFloat32*;

//
// Ren3dShaderVarFloat32
// ==========================================================================


// ==========================================================================
// Ren3dShaderVarVec2
//

class Ren3dShaderVarVec2 :
	public virtual Ren3dShaderVar
{
protected:
	Ren3dShaderVarVec2() = default;

	virtual ~Ren3dShaderVarVec2() = default;


public:
	virtual void set_vec2(
		const float* const value) = 0;
}; // Ren3dShaderVarVec2

using Ren3dShaderVec2VarPtr = Ren3dShaderVarVec2*;

//
// Ren3dShaderVarVec2
// ==========================================================================


// ==========================================================================
// Ren3dShaderVarVec4
//

class Ren3dShaderVarVec4 :
	public virtual Ren3dShaderVar
{
protected:
	Ren3dShaderVarVec4() = default;

	virtual ~Ren3dShaderVarVec4() = default;


public:
	virtual void set_vec4(
		const float* const value) = 0;
}; // Ren3dShaderVarVec4

using Ren3dShaderVec4VarPtr = Ren3dShaderVarVec4*;

//
// Ren3dShaderVarVec4
// ==========================================================================


// ==========================================================================
// Ren3dShaderVarMat4
//

class Ren3dShaderVarMat4 :
	public virtual Ren3dShaderVar
{
protected:
	Ren3dShaderVarMat4() = default;

	virtual ~Ren3dShaderVarMat4() = default;


public:
	virtual void set_mat4(
		const float* const value) = 0;
}; // Ren3dShaderVarMat4

using Ren3dShaderMat4VarPtr = Ren3dShaderVarMat4*;

//
// Ren3dShaderVarMat4
// ==========================================================================


// ==========================================================================
// Ren3dShaderVarSampler2d
//

class Ren3dShaderVarSampler2d :
	public virtual Ren3dShaderVar
{
protected:
	Ren3dShaderVarSampler2d() = default;

	virtual ~Ren3dShaderVarSampler2d() = default;


public:
	virtual void set_sampler_2d(
		const std::int32_t value) = 0;
}; // Ren3dShaderVarSampler2d

using Ren3dShaderSampler2dVarPtr = Ren3dShaderVarSampler2d*;

//
// Ren3dShaderVarSampler2d
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_SHADER_VAR_INCLUDED
