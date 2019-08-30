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
// OpenGL context.
//


#ifndef BSTONE_DETAIL_OGL_CONTEXT_INCLUDED
#define BSTONE_DETAIL_OGL_CONTEXT_INCLUDED


#include "bstone_detail_ogl_buffer.h"
#include "bstone_detail_ogl_device_features.h"
#include "bstone_detail_ogl_vertex_input.h"


namespace bstone
{
namespace detail
{


class OglVao;
using OglVaoPtr = OglVao*;

class OglBufferManager;
using OglBufferManagerPtr = OglBufferManager*;

class OglSamplerManager;
using OglSamplerManagerPtr = OglSamplerManager*;

class OglTextureManager;
using OglTextureManagerPtr = OglTextureManager*;

class OglVaoManager;
using OglVaoManagerPtr = OglVaoManager*;

class OglVertexInputManager;
using OglVertexInputManagerPtr = OglVertexInputManager*;

class OglShaderManager;
using OglShaderManagerPtr = OglShaderManager*;

class OglShaderStageManager;
using OglShaderStageManagerPtr = OglShaderStageManager*;


// =========================================================================
// OglContext
//

class OglContext
{
public:
	OglContext();

	OglContext(
		const OglContext& rhs) = delete;

	virtual ~OglContext() = 0;


	virtual const RendererDeviceFeatures& get_device_features() const noexcept = 0;

	virtual const OglDeviceFeatures& get_ogl_device_features() const noexcept = 0;


	virtual OglBufferManagerPtr buffer_get_manager() const noexcept = 0;

	virtual OglSamplerManagerPtr sampler_get_manager() const noexcept = 0;

	virtual OglTextureManagerPtr texture_get_manager() const noexcept = 0;

	virtual OglVaoManagerPtr vao_get_manager() const noexcept = 0;

	virtual OglVertexInputManagerPtr vertex_input_get_manager() const noexcept = 0;

	virtual OglShaderManagerPtr shader_get_manager() const noexcept = 0;

	virtual OglShaderStageManagerPtr shader_stage_get_manager() const noexcept = 0;
}; // OglContext


using OglContextPtr = OglContext*;
using OglContextUPtr = std::unique_ptr<OglContext>;

//
// OglContext
// =========================================================================


// =========================================================================
// OglContextFactory
//


struct OglContextFactory
{
	static OglContextUPtr create(
		const RendererKind renderer_kind,
		const RendererDeviceFeatures& device_features,
		const OglDeviceFeatures& ogl_device_features);
}; // OglContextFactory


//
// OglContextFactory
// =========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_CONTEXT_INCLUDED
