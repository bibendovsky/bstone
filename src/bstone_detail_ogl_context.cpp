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
#include "bstone_detail_ogl_vertex_input.h"
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


	void viewport_set(
		const RendererViewport& viewport) override;


	void scissor_enable(
		const bool is_enable) override;

	void scissor_set_box(
		const RendererScissorBox& scissor_box) override;


	void culling_enable(
		const bool is_enable) override;


	void depth_test_enable(
		const bool is_enable) override;

	void depth_write_enable(
		const bool is_enable) override;


	void blending_enable(
		const bool is_enable) override;

	void blending_set_func(
		const RendererBlendingFunc& func) override;


private:
	const RendererDeviceFeatures& device_features_;
	const OglDeviceFeatures& ogl_device_features_;

	RendererViewport viewport_;

	bool scissor_is_enabled_;
	RendererScissorBox scissor_box_;

	bool culling_is_enabled_;
	RendererCullingFace culling_face_;
	RendererCullingMode culling_mode_;

	bool depth_is_test_enabled_;
	bool depth_is_write_enabled_;

	bool blending_is_enabled_;
	RendererBlendingFunc blending_func_;

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


	void viewport_set_rect();

	void viewport_set_depth_range();

	void viewport_set_defaults();


	void scissor_enable();

	void scissor_set_box();

	void scissor_set_defaults();


	void culling_enable();

	void culling_set_face();

	void culling_set_mode();

	void culling_set_defaults();


	void depth_set_test();

	void depth_set_write();

	void depth_set_defaults();


	void blending_enable();

	void blending_set_func();

	void blending_set_defaults();
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
	viewport_{},
	scissor_is_enabled_{},
	scissor_box_{},
	culling_is_enabled_{},
	culling_face_{},
	culling_mode_{},
	depth_is_test_enabled_{},
	depth_is_write_enabled_{},
	blending_is_enabled_{},
	blending_func_{},
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

	viewport_set_defaults();
	scissor_set_defaults();
	culling_set_defaults();
	depth_set_defaults();
	blending_set_defaults();
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

void GenericOglContext::viewport_set(
	const RendererViewport& viewport)
{
	if (viewport_.x_ != viewport.x_ ||
		viewport_.y_ != viewport.y_ ||
		viewport_.width_ != viewport.width_ ||
		viewport_.height_ != viewport.height_)
	{
		viewport_.x_ = viewport.x_;
		viewport_.y_ = viewport.y_;
		viewport_.width_ = viewport.width_;
		viewport_.height_ = viewport.height_;
		viewport_set_rect();
	}

	if (viewport_.min_depth_ != viewport.min_depth_ ||
		viewport_.max_depth_ != viewport.max_depth_)
	{
		viewport_.min_depth_ = viewport.min_depth_;
		viewport_.max_depth_ = viewport.max_depth_;
		viewport_set_depth_range();
	}
}

void GenericOglContext::scissor_enable(
	const bool is_enable)
{
	if (scissor_is_enabled_ != is_enable)
	{
		scissor_is_enabled_ = is_enable;
		scissor_enable();
	}
}

void GenericOglContext::scissor_set_box(
	const RendererScissorBox& scissor_box)
{
	if (scissor_box_.x_ != scissor_box.x_ ||
		scissor_box_.y_ != scissor_box.y_ ||
		scissor_box_.width_ != scissor_box.width_ ||
		scissor_box_.height_ != scissor_box.height_)
	{
		scissor_box_ = scissor_box;
		scissor_set_box();
	}
}

void GenericOglContext::culling_enable(
	const bool is_enable)
{
	if (culling_is_enabled_ != is_enable)
	{
		culling_is_enabled_ = is_enable;
		culling_enable();
	}
}

void GenericOglContext::depth_test_enable(
	const bool is_enable)
{
	if (depth_is_test_enabled_ != is_enable)
	{
		depth_is_test_enabled_ = is_enable;
		depth_set_test();
	}
}

void GenericOglContext::depth_write_enable(
	const bool is_enable)
{
	if (depth_is_write_enabled_ != is_enable)
	{
		depth_is_write_enabled_ = is_enable;
		depth_set_write();
	}
}


void GenericOglContext::blending_enable(
	const bool is_enable)
{
	if (blending_is_enabled_ != is_enable)
	{
		blending_is_enabled_ = is_enable;
		blending_enable();
	}
}

void GenericOglContext::blending_set_func(
	const RendererBlendingFunc& blending_func)
{
	if (blending_func_.src_factor_ != blending_func.src_factor_ ||
		blending_func_.dst_factor_ != blending_func.dst_factor_)
	{
		blending_func_ = blending_func;
		blending_set_func();
	}
}

void GenericOglContext::viewport_set_rect()
{
	OglRendererUtils::viewport_set_rect(viewport_);
}

void GenericOglContext::viewport_set_depth_range()
{
	OglRendererUtils::viewport_set_depth_range(viewport_);
}

void GenericOglContext::viewport_set_defaults()
{
	viewport_.x_ = 0;
	viewport_.y_ = 0;
	viewport_.width_ = 0;
	viewport_.height_ = 0;
	viewport_set_rect();

	viewport_.min_depth_ = 0.0F;
	viewport_.max_depth_ = 1.0F;
	viewport_set_depth_range();
}

void GenericOglContext::scissor_enable()
{
	OglRendererUtils::scissor_enable(scissor_is_enabled_);
}

void GenericOglContext::scissor_set_box()
{
	OglRendererUtils::scissor_set_box(scissor_box_);
}

void GenericOglContext::scissor_set_defaults()
{
	scissor_is_enabled_ = false;
	scissor_enable();

	scissor_box_.x_ = 0;
	scissor_box_.y_ = 0;
	scissor_box_.width_ = 0;
	scissor_box_.height_ = 0;
	scissor_set_box();
}

void GenericOglContext::culling_enable()
{
	OglRendererUtils::culling_enable(culling_is_enabled_);
}

void GenericOglContext::culling_set_face()
{
	OglRendererUtils::culling_set_face(culling_face_);
}

void GenericOglContext::culling_set_mode()
{
	OglRendererUtils::culling_set_mode(culling_mode_);
}

void GenericOglContext::culling_set_defaults()
{
	culling_is_enabled_ = false;
	culling_enable();

	culling_face_ = RendererCullingFace::counter_clockwise;
	culling_set_face();

	culling_mode_ = RendererCullingMode::back;
	culling_set_mode();
}

void GenericOglContext::depth_set_test()
{
	OglRendererUtils::depth_test_enable(depth_is_test_enabled_);
}

void GenericOglContext::depth_set_write()
{
	OglRendererUtils::depth_write_enable(depth_is_write_enabled_);
}

void GenericOglContext::depth_set_defaults()
{
	depth_is_test_enabled_ = false;
	depth_set_test();

	depth_is_write_enabled_ = false;
	depth_set_write();
}

void GenericOglContext::blending_enable()
{
	OglRendererUtils::blending_enable(blending_is_enabled_);
}

void GenericOglContext::blending_set_func()
{
	OglRendererUtils::blending_set_func(blending_func_);
}

void GenericOglContext::blending_set_defaults()
{
	blending_is_enabled_ = false;
	blending_enable();

	blending_func_.src_factor_ = RendererBlendingFactor::src_alpha;
	blending_func_.dst_factor_ = RendererBlendingFactor::one_minus_src_alpha;
	blending_set_func();
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
