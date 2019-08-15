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
// Renderer (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_renderer.h"
#include "bstone_detail_renderer_command_manager.h"


namespace bstone
{


// ==========================================================================
// RendererBuffer
//

RendererBuffer::RendererBuffer() = default;

RendererBuffer::~RendererBuffer() = default;

//
// RendererBuffer
// ==========================================================================


// ==========================================================================
// RendererIndexBuffer
//

RendererIndexBuffer::RendererIndexBuffer() = default;

RendererIndexBuffer::~RendererIndexBuffer() = default;

//
// RendererIndexBuffer
// ==========================================================================


// ==========================================================================
// RendererVertexBuffer
//

RendererVertexBuffer::RendererVertexBuffer() = default;

RendererVertexBuffer::~RendererVertexBuffer() = default;

//
// RendererVertexBuffer
// ==========================================================================


// ==========================================================================
// Shader
//

RendererShaderVariable::RendererShaderVariable() = default;

RendererShaderVariable::~RendererShaderVariable() = default;


RendererShaderVariableInt32::RendererShaderVariableInt32() = default;

RendererShaderVariableInt32::~RendererShaderVariableInt32() = default;


RendererShaderVariableFloat32::RendererShaderVariableFloat32() = default;

RendererShaderVariableFloat32::~RendererShaderVariableFloat32() = default;


RendererShaderVariableVec2::RendererShaderVariableVec2() = default;

RendererShaderVariableVec2::~RendererShaderVariableVec2() = default;


RendererShaderVariableVec4::RendererShaderVariableVec4() = default;

RendererShaderVariableVec4::~RendererShaderVariableVec4() = default;


RendererShaderVariableMat4::RendererShaderVariableMat4() = default;

RendererShaderVariableMat4::~RendererShaderVariableMat4() = default;


RendererShader::RendererShader() = default;

RendererShader::~RendererShader() = default;


RendererShaderStage::RendererShaderStage() = default;

RendererShaderStage::~RendererShaderStage() = default;

//
// Shader
// ==========================================================================


// ==========================================================================
// RendererCommandManagerFactory
//

RendererCommandManagerUPtr RendererCommandManagerFactory::create()
{
	return detail::RendererCommandManagerUPtr{new detail::RendererCommandManager{}};
}

//
// RendererCommandManagerFactory
// ==========================================================================


} // bstone
