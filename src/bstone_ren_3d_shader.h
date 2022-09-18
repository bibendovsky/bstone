/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// 3D renderer's shader.
//


#ifndef BSTONE_REN_3D_SHADER_INCLUDED
#define BSTONE_REN_3D_SHADER_INCLUDED


#include <memory>


namespace bstone
{


// ==========================================================================
// Ren3dShader
//

enum class Ren3dShaderKind
{
	none,
	fragment,
	vertex,
}; // Ren3dShaderKind

struct Ren3dShaderSource
{
	const void* data_;
	int size_;
}; // Source

struct Ren3dCreateShaderParam
{
	Ren3dShaderKind kind;
	Ren3dShaderSource source_;
}; // CreateParam


class Ren3dShader
{
public:
	Ren3dShader() noexcept = default;

	virtual ~Ren3dShader() = default;


	virtual Ren3dShaderKind get_kind() const noexcept = 0;
}; // Ren3dShader

using Ren3dShaderPtr = Ren3dShader*;
using Ren3dShaderUPtr = std::unique_ptr<Ren3dShader>;

//
// Ren3dShader
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_SHADER_INCLUDED
