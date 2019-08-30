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
// OpenGL context (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_context.h"

#include <list>
#include <vector>

#include "bstone_exception.h"
#include "bstone_ogl_unique_resources.h"

#include "bstone_detail_ogl_buffer.h"
#include "bstone_detail_ogl_buffer_manager.h"
#include "bstone_detail_ogl_device_features.h"
#include "bstone_detail_ogl_renderer_utils.h"
#include "bstone_detail_ogl_sampler.h"
#include "bstone_detail_ogl_sampler_manager.h"
#include "bstone_detail_ogl_shader_manager.h"
#include "bstone_detail_ogl_shader_stage_manager.h"
#include "bstone_detail_ogl_texture_2d.h"
#include "bstone_detail_ogl_texture_manager.h"
#include "bstone_detail_ogl_vao_manager.h"
#include "bstone_detail_ogl_vertex_input_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglContext
//

OglContext::OglContext() = default;

OglContext::~OglContext() = default;

//
// OglContext
// ==========================================================================


// ==========================================================================
// GenericOglContext
//

class GenericOglContext :
	public OglContext
{
public:
	GenericOglContext(
		const RendererDeviceFeatures& device_features,
		const OglDeviceFeatures& ogl_device_features);

	GenericOglContext(
		const GenericOglContext& rhs) = delete;

	~GenericOglContext() override;


	const RendererDeviceFeatures& get_device_features() const noexcept override;

	const OglDeviceFeatures& get_ogl_device_features() const noexcept override;


	OglBufferManagerPtr buffer_get_manager() const noexcept override;

	OglSamplerManagerPtr sampler_get_manager() const noexcept override;

	OglTextureManagerPtr texture_get_manager() const noexcept;

	OglVaoManagerPtr vao_get_manager() const noexcept override;

	OglVertexInputManagerPtr vertex_input_get_manager() const noexcept override;

	OglShaderManagerPtr shader_get_manager() const noexcept override;

	OglShaderStageManagerPtr shader_stage_get_manager() const noexcept override;


private:
	const RendererDeviceFeatures& device_features_;
	const OglDeviceFeatures& ogl_device_features_;

	OglVaoManagerUPtr vao_manager_;
	OglBufferManagerUPtr buffer_manager_;
	OglVertexInputManagerUPtr vertex_input_manager_;
	OglSamplerManagerUPtr sampler_manager_;
	OglTextureManagerUPtr texture_manager_;
	OglShaderManagerUPtr shader_manager_;
	OglShaderStageManagerUPtr shader_stage_manager_;


	void initialize();

	void initialize_vertex_arrays();

	void initialize_buffers();

	void initialize_vertex_inputs();

	void initialize_samplers();

	void initialize_textures();

	void initialize_shaders();

	void initialize_shader_stages();

	void mipmap_set_max_quality();
}; // GenericOglContext

using GenericOglContextPtr = GenericOglContext*;
using GenericOglContextUPtr = std::unique_ptr<GenericOglContext>;

//
// GenericOglContext
// ==========================================================================


// ==========================================================================
// GenericOglContext
//

GenericOglContext::GenericOglContext(
	const RendererDeviceFeatures& device_features,
	const OglDeviceFeatures& ogl_device_features)
	:
	device_features_{device_features},
	ogl_device_features_{ogl_device_features},
	vao_manager_{},
	buffer_manager_{},
	vertex_input_manager_{},
	sampler_manager_{},
	texture_manager_{}
{
	initialize();
}

GenericOglContext::~GenericOglContext() = default;

void GenericOglContext::initialize_vertex_arrays()
{
	vao_manager_ = OglVaoManagerFactory::create(this, device_features_, ogl_device_features_);
}

void GenericOglContext::initialize_buffers()
{
	buffer_manager_ = OglBufferManagerFactory::create(this, vao_manager_.get());
}

void GenericOglContext::initialize_vertex_inputs()
{
	vertex_input_manager_ = OglVertexInputManagerFactory::create(this);
}

void GenericOglContext::initialize_samplers()
{
	sampler_manager_ = OglSamplerManagerFactory::create(this);
}

void GenericOglContext::initialize_textures()
{
	texture_manager_ = OglTextureManagerFactory::create(this);
}

void GenericOglContext::initialize_shaders()
{
	shader_manager_ = OglShaderManagerFactory::create(this);
}

void GenericOglContext::initialize_shader_stages()
{
	shader_stage_manager_ = OglShaderStageManagerFactory::create(this);
}

void GenericOglContext::mipmap_set_max_quality()
{
	if (!device_features_.mipmap_is_available_)
	{
		return;
	}

	if (ogl_device_features_.context_kind_ == OglContextKind::core)
	{
		return;
	}

	::glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	assert(!OglRendererUtils::was_errors());
}

void GenericOglContext::initialize()
{
	mipmap_set_max_quality();
	initialize_vertex_arrays();
	initialize_buffers();
	initialize_vertex_inputs();
	initialize_samplers();
	initialize_textures();
	initialize_shaders();
	initialize_shader_stages();
}

const RendererDeviceFeatures& GenericOglContext::get_device_features() const noexcept
{
	return device_features_;
}

const OglDeviceFeatures& GenericOglContext::get_ogl_device_features() const noexcept
{
	return ogl_device_features_;
}

OglBufferManagerPtr GenericOglContext::buffer_get_manager() const noexcept
{
	return buffer_manager_.get();
}

OglSamplerManagerPtr GenericOglContext::sampler_get_manager() const noexcept
{
	return sampler_manager_.get();
}

OglTextureManagerPtr GenericOglContext::texture_get_manager() const noexcept
{
	return texture_manager_.get();
}

OglVaoManagerPtr GenericOglContext::vao_get_manager() const noexcept
{
	return vao_manager_.get();
}

OglVertexInputManagerPtr GenericOglContext::vertex_input_get_manager() const noexcept
{
	return vertex_input_manager_.get();
}

OglShaderManagerPtr GenericOglContext::shader_get_manager() const noexcept
{
	return shader_manager_.get();
}

OglShaderStageManagerPtr GenericOglContext::shader_stage_get_manager() const noexcept
{
	return shader_stage_manager_.get();
}

//
// GenericOglContext
// ==========================================================================


// =========================================================================
// OglContextFactory
//

OglContextUPtr OglContextFactory::create(
	const RendererKind renderer_kind,
	const RendererDeviceFeatures& device_features,
	const OglDeviceFeatures& ogl_device_features)
{
	switch (renderer_kind)
	{
		case RendererKind::ogl_2_x:
			return GenericOglContextUPtr{new GenericOglContext{device_features, ogl_device_features}};

		default:
			throw Exception{"Unsupported renderer kind."};
	}
}

//
// OglContextFactory
// =========================================================================


} // detail
} // bstone
