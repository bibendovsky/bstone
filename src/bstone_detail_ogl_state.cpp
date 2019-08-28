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
// OpenGL state (implementation).
//


#include "bstone_precompiled.h"
#include "bstone_detail_ogl_state.h"

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
#include "bstone_detail_ogl_texture_2d.h"
#include "bstone_detail_ogl_texture_manager.h"
#include "bstone_detail_ogl_vao_manager.h"
#include "bstone_detail_ogl_vertex_input_manager.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// OglState
//

OglState::OglState() = default;

OglState::~OglState() = default;

//
// OglState
// ==========================================================================


// ==========================================================================
// GenericOglState
//

class GenericOglState :
	public OglState
{
public:
	GenericOglState(
		const RendererDeviceFeatures& device_features,
		const OglDeviceFeatures& ogl_device_features);

	GenericOglState(
		const GenericOglState& rhs) = delete;

	~GenericOglState() override;


	const RendererDeviceFeatures& get_device_features() const noexcept override;

	const OglDeviceFeatures& get_ogl_device_features() const noexcept override;


	RendererIndexBufferPtr index_buffer_create(
		const RendererIndexBufferCreateParam& param) override;

	RendererVertexBufferPtr vertex_buffer_create(
		const RendererVertexBufferCreateParam& param) override;


	void buffer_destroy(
		const RendererBufferPtr buffer) override;

	bool buffer_set_current(
		const RendererBufferKind buffer_kind,
		const RendererBufferPtr buffer) override;


	OglSamplerManagerPtr sampler_get_manager() const noexcept override;

	RendererSamplerPtr sampler_create(
		const RendererSamplerCreateParam& param) override;

	void sampler_destroy(
		const RendererSamplerPtr sampler) override;

	void sampler_set(
		const RendererSamplerPtr sampler) override;


	OglTextureManagerPtr texture_manager_get() const noexcept;

	RendererTexture2dPtr texture_2d_create(
		const RendererTexture2dCreateParam& param) override;

	void texture_2d_destroy(
		const RendererTexture2dPtr texture_2d) override;

	void texture_2d_set(
		const RendererTexture2dPtr texture_2d) override;


	OglVaoManagerPtr vao_get_manager() const noexcept override;

	OglVaoPtr vao_create() override;

	void vao_destroy(
		const OglVaoPtr vao) override;

	void vao_bind(
		const OglVaoPtr vao) override;

	void vao_push_current_set_default() override;

	void vao_pop() override;


	OglVertexInputManagerPtr vertex_input_get_manager() const noexcept override;

	RendererVertexInputPtr vertex_input_create(
		const RendererVertexInputCreateParam& param) override;

	void vertex_input_destroy(
		const RendererVertexInputPtr vertex_input) override;

	void vertex_input_set(
		const RendererVertexInputPtr vertex_input) override;

	RendererIndexBufferPtr vertex_input_get_index_buffer() const noexcept override;


private:
	const RendererDeviceFeatures& device_features_;
	const OglDeviceFeatures& ogl_device_features_;

	OglVaoManagerUPtr vao_manager_;
	OglBufferManagerUPtr buffer_manager_;
	OglVertexInputManagerUPtr vertex_input_manager_;
	OglSamplerManagerUPtr sampler_manager_;
	OglTextureManagerUPtr texture_manager_;


	void initialize();

	void initialize_vertex_arrays();

	void initialize_buffers();

	void initialize_vertex_inputs();

	void initialize_samplers();

	void initialize_textures();

	void mipmap_set_max_quality();
}; // GenericOglState

using GenericOglStatePtr = GenericOglState*;
using GenericOglStateUPtr = std::unique_ptr<GenericOglState>;

//
// GenericOglState
// ==========================================================================


// ==========================================================================
// GenericOglState
//

GenericOglState::GenericOglState(
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

GenericOglState::~GenericOglState() = default;

void GenericOglState::initialize_vertex_arrays()
{
	vao_manager_ = OglVaoManagerFactory::create(this, device_features_, ogl_device_features_);
}

void GenericOglState::initialize_buffers()
{
	buffer_manager_ = OglBufferManagerFactory::create(this, vao_manager_.get());
}

void GenericOglState::initialize_vertex_inputs()
{
	vertex_input_manager_ = OglVertexInputManagerFactory::create(this);
}

void GenericOglState::initialize_samplers()
{
	sampler_manager_ = OglSamplerManagerFactory::create(this);
}

void GenericOglState::initialize_textures()
{
	texture_manager_ = OglTextureManagerFactory::create(this);
}

void GenericOglState::mipmap_set_max_quality()
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

void GenericOglState::initialize()
{
	mipmap_set_max_quality();
	initialize_vertex_arrays();
	initialize_buffers();
	initialize_vertex_inputs();
	initialize_samplers();
	initialize_textures();
}

const RendererDeviceFeatures& GenericOglState::get_device_features() const noexcept
{
	return device_features_;
}

const OglDeviceFeatures& GenericOglState::get_ogl_device_features() const noexcept
{
	return ogl_device_features_;
}

RendererIndexBufferPtr GenericOglState::index_buffer_create(
	const RendererIndexBufferCreateParam& param)
{
	return buffer_manager_->index_buffer_create(param);
}

RendererVertexBufferPtr GenericOglState::vertex_buffer_create(
	const RendererVertexBufferCreateParam& param)
{
	return buffer_manager_->vertex_buffer_create(param);
}

void GenericOglState::buffer_destroy(
	const RendererBufferPtr buffer)
{
	buffer_manager_->buffer_destroy(buffer);
}

bool GenericOglState::buffer_set_current(
	const RendererBufferKind buffer_kind,
	const RendererBufferPtr buffer)
{
	return buffer_manager_->buffer_set_current(buffer_kind, buffer);
}

OglSamplerManagerPtr GenericOglState::sampler_get_manager() const noexcept
{
	return sampler_manager_.get();
}

RendererSamplerPtr GenericOglState::sampler_create(
	const RendererSamplerCreateParam& param)
{
	return sampler_manager_->sampler_create(param);
}

void GenericOglState::sampler_destroy(
	const RendererSamplerPtr sampler)
{
	sampler_manager_->sampler_destroy(sampler);
}

void GenericOglState::sampler_set(
	const RendererSamplerPtr sampler)
{
	sampler_manager_->sampler_set(sampler);
}

OglTextureManagerPtr GenericOglState::texture_manager_get() const noexcept
{
	return texture_manager_.get();
}

RendererTexture2dPtr GenericOglState::texture_2d_create(
	const RendererTexture2dCreateParam& param)
{
	return texture_manager_->texture_2d_create(param);
}

void GenericOglState::texture_2d_destroy(
	const RendererTexture2dPtr texture_2d)
{
	texture_manager_->texture_2d_destroy(texture_2d);
}

void GenericOglState::texture_2d_set(
	const RendererTexture2dPtr texture_2d)
{
	texture_manager_->texture_2d_set(texture_2d);
}

OglVaoManagerPtr GenericOglState::vao_get_manager() const noexcept
{
	return vao_manager_.get();
}

OglVaoPtr GenericOglState::vao_create()
{
	return vao_manager_->create();
}

void GenericOglState::vao_destroy(
	const OglVaoPtr vao)
{
	vao_manager_->destroy(vao);
}

void GenericOglState::vao_bind(
	const OglVaoPtr vao)
{
	if (!vao)
	{
		throw Exception{"Null VAO."};
	}

	vao_manager_->bind(vao);
}

void GenericOglState::vao_push_current_set_default()
{
	vao_manager_->push_current_set_default();
}

void GenericOglState::vao_pop()
{
	vao_manager_->pop();
}

OglVertexInputManagerPtr GenericOglState::vertex_input_get_manager() const noexcept
{
	return vertex_input_manager_.get();
}

RendererVertexInputPtr GenericOglState::vertex_input_create(
	const RendererVertexInputCreateParam& param)
{
	return vertex_input_manager_->vertex_input_create(param);
}

void GenericOglState::vertex_input_destroy(
	const RendererVertexInputPtr vertex_input)
{
	vertex_input_manager_->vertex_input_destroy(vertex_input);
}

void GenericOglState::vertex_input_set(
	const RendererVertexInputPtr vertex_input)
{
	vertex_input_manager_->vertex_input_set(vertex_input);
}

RendererIndexBufferPtr GenericOglState::vertex_input_get_index_buffer() const noexcept
{
	const auto vertex_input = static_cast<OglVertexInputPtr>(vertex_input_manager_->vertex_input_get_current());

	if (!vertex_input)
	{
		return nullptr;
	}

	return vertex_input->get_index_buffer();
}

//
// GenericOglState
// ==========================================================================


// =========================================================================
// OglStateFactory
//

OglStateUPtr OglStateFactory::create(
	const RendererKind renderer_kind,
	const RendererDeviceFeatures& device_features,
	const OglDeviceFeatures& ogl_device_features)
{
	switch (renderer_kind)
	{
		case RendererKind::ogl_2_x:
			return GenericOglStateUPtr{new GenericOglState{device_features, ogl_device_features}};

		default:
			throw Exception{"Unsupported renderer kind."};
	}
}

//
// OglStateFactory
// =========================================================================


} // detail
} // bstone
