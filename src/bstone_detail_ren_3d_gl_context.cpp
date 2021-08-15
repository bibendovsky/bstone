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
// OpenGL context (implementation).
//


#include "bstone_detail_ren_3d_gl_context.h"

#include <list>
#include <vector>

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"

#include "bstone_detail_ren_3d_gl_buffer.h"
#include "bstone_detail_ren_3d_gl_device_features.h"
#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_utils.h"
#include "bstone_detail_ren_3d_gl_sampler.h"
#include "bstone_detail_ren_3d_gl_sampler_mgr.h"
#include "bstone_detail_ren_3d_gl_shader.h"
#include "bstone_detail_ren_3d_gl_shader_stage_mgr.h"
#include "bstone_detail_ren_3d_gl_texture_2d.h"
#include "bstone_detail_ren_3d_gl_texture_mgr.h"
#include "bstone_detail_ren_3d_gl_vertex_input.h"
#include "bstone_detail_ren_3d_gl_vertex_input_mgr.h"


namespace bstone
{
namespace detail
{


// ==========================================================================
// Ren3dGlContextException

class Ren3dGlContextException :
	public Exception
{
public:
	explicit Ren3dGlContextException(
		const char* message) noexcept
		:
		Exception{"REN_3D_GL_CTX", message}
	{
	}
}; // Ren3dGlContextException
// Ren3dGlContextException
// ==========================================================================


// ==========================================================================
// Ren3dGlContextImpl
//

class Ren3dGlContextImpl final :
	public Ren3dGlContext
{
public:
	Ren3dGlContextImpl(
		const Ren3dDeviceFeatures& device_features,
		const Ren3dGlDeviceFeatures& gl_device_features);

	Ren3dGlContextImpl(
		const Ren3dGlContextImpl& rhs) = delete;

	~Ren3dGlContextImpl() override;


	const Ren3dDeviceFeatures& get_device_features() const noexcept override;

	const Ren3dGlDeviceFeatures& get_gl_device_features() const noexcept override;


	Ren3dGlBufferUPtr create_buffer(
		const Ren3dCreateBufferParam& param) override;

	Ren3dGlSamplerMgrPtr get_sampler_manager() const noexcept override;

	Ren3dGlTextureMgrPtr get_texture_manager() const noexcept override;

	Ren3dGlVertexInputMgrPtr get_vertex_input_manager() const noexcept override;

	Ren3dGlShaderUPtr create_shader(
		const Ren3dCreateShaderParam& param) override;

	Ren3dGlShaderStageMgrPtr get_shader_stage_manager() const noexcept override;


	Ren3dGlSamplerPtr get_sampler() const noexcept override;

	void set_sampler(
		const Ren3dGlSamplerPtr sampler) noexcept override;


	Ren3dGlTexture2dPtr get_texture_2d() const noexcept override;

	void set_texture_2d(
		const Ren3dGlTexture2dPtr texture_2d) noexcept override;


	Ren3dGlVertexInputPtr get_vertex_input() const noexcept override;

	void set_vertex_input(
		const Ren3dGlVertexInputPtr vertex_input) noexcept override;


	Ren3dGlShaderStagePtr get_shader_stage() const noexcept override;

	void set_shader_stage(
		const Ren3dGlShaderStagePtr shader_stage) noexcept override;


	void clear(
		const Rgba8& rgba_8) override;

	void set_viewport(
		const Ren3dViewport& viewport) override;


	void enable_scissor(
		const bool is_enable) override;

	void set_scissor_box(
		const Ren3dScissorBox& scissor_box) override;


	void enable_culling(
		const bool is_enable) override;


	void enable_depth_test(
		const bool is_enable) override;

	void enable_depth_write(
		const bool is_enable) override;


	void enable_blending(
		const bool is_enable) override;

	void set_blending_func(
		const Ren3dBlendingFunc& func) override;


private:
	const Ren3dDeviceFeatures& device_features_;
	const Ren3dGlDeviceFeatures& gl_device_features_;

	Rgba8 clear_color_;

	Ren3dViewport viewport_;

	bool is_scissor_enabled_;
	Ren3dScissorBox scissor_box_;

	bool is_culling_enabled_;
	Ren3dCullingFace culling_face_;
	Ren3dCullingMode culling_mode_;

	bool is_depth_test_enabled_;
	bool is_depth_write_enabled_;

	bool is_blending_enabled_;
	Ren3dBlendingFunc blending_func_;

	Ren3dGlVertexInputMgrUPtr vertex_input_manager_;
	Ren3dGlSamplerMgrUPtr sampler_manager_;
	Ren3dGlTextureMgrUPtr texture_manager_;
	Ren3dGlShaderStageMgrUPtr shader_stage_manager_;

	Ren3dGlSamplerPtr sampler_;
	Ren3dGlTexture2dPtr texture_2d_;
	Ren3dGlVertexInputPtr vertex_input_;
	Ren3dGlShaderStagePtr shader_stage_;


	[[noreturn]]
	static void fail(
		const char* message);

	[[noreturn]]
	static void fail_nested(
		const char* message);


	void set_max_mipmap_quality();


	void set_clear_color();

	void clear();

	void set_clear_defaults();


	void set_viewport_rect();

	void set_viewport_depth_range();

	void set_viewport_defaults();


	void enable_scissor();

	void set_scissor_box();

	void set_scissor_defaults();


	void enable_culling();

	void set_culling_face();

	void set_culling_mode();

	void set_culling_defaults();


	void enable_depth_test();

	void enable_depth_write();

	void set_depth_defaults();


	void enable_blending();

	void set_blending_func();

	void set_blending_defaults();
}; // Ren3dGlContextImpl

//
// Ren3dGlContextImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlContextImpl
//

Ren3dGlContextImpl::Ren3dGlContextImpl(
	const Ren3dDeviceFeatures& device_features,
	const Ren3dGlDeviceFeatures& gl_device_features)
try
	:
	device_features_{device_features},
	gl_device_features_{gl_device_features},
	clear_color_{},
	viewport_{},
	is_scissor_enabled_{},
	scissor_box_{},
	is_culling_enabled_{},
	culling_face_{},
	culling_mode_{},
	is_depth_test_enabled_{},
	is_depth_write_enabled_{},
	is_blending_enabled_{},
	blending_func_{},
	vertex_input_manager_{Ren3dGlVertexInputMgrFactory::create(this)},
	sampler_manager_{Ren3dGlSamplerMgrFactory::create(this)},
	texture_manager_{Ren3dGlTextureMgrFactory::create(this)},
	shader_stage_manager_{Ren3dGlShaderStageMgrFactory::create(this)},
	sampler_{},
	texture_2d_{},
	vertex_input_{},
	shader_stage_{}
{
	set_max_mipmap_quality();

	set_clear_defaults();
	set_viewport_defaults();
	set_scissor_defaults();
	set_culling_defaults();
	set_depth_defaults();
	set_blending_defaults();
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlContextImpl::~Ren3dGlContextImpl() = default;

[[noreturn]]
void Ren3dGlContextImpl::fail(
	const char* message)
{
	throw Ren3dGlContextException{message};
}

[[noreturn]]
void Ren3dGlContextImpl::fail_nested(
	const char* message)
{
	std::throw_with_nested(Ren3dGlContextException{message});
}

void Ren3dGlContextImpl::set_max_mipmap_quality()
try
{
	if (!device_features_.is_mipmap_available_)
	{
		return;
	}

	if (gl_device_features_.context_kind_ == Ren3dGlContextKind::core)
	{
		return;
	}

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

const Ren3dDeviceFeatures& Ren3dGlContextImpl::get_device_features() const noexcept
{
	return device_features_;
}

const Ren3dGlDeviceFeatures& Ren3dGlContextImpl::get_gl_device_features() const noexcept
{
	return gl_device_features_;
}

Ren3dGlBufferUPtr Ren3dGlContextImpl::create_buffer(
	const Ren3dCreateBufferParam& param)
try
{
	return Ren3dGlBufferFactory::create(this, param);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlSamplerMgrPtr Ren3dGlContextImpl::get_sampler_manager() const noexcept
{
	return sampler_manager_.get();
}

Ren3dGlTextureMgrPtr Ren3dGlContextImpl::get_texture_manager() const noexcept
{
	return texture_manager_.get();
}

Ren3dGlVertexInputMgrPtr Ren3dGlContextImpl::get_vertex_input_manager() const noexcept
{
	return vertex_input_manager_.get();
}

Ren3dGlShaderUPtr Ren3dGlContextImpl::create_shader(
	const Ren3dCreateShaderParam& param)
try
{
	return Ren3dGlShaderFactory::create(param);
}
catch (...)
{
	fail_nested(__func__);
}

Ren3dGlShaderStageMgrPtr Ren3dGlContextImpl::get_shader_stage_manager() const noexcept
{
	return shader_stage_manager_.get();
}

Ren3dGlSamplerPtr Ren3dGlContextImpl::get_sampler() const noexcept
{
	return sampler_;
}

void Ren3dGlContextImpl::set_sampler(
	const Ren3dGlSamplerPtr sampler) noexcept
{
	sampler_ = sampler;
}

Ren3dGlTexture2dPtr Ren3dGlContextImpl::get_texture_2d() const noexcept
{
	return texture_2d_;
}

void Ren3dGlContextImpl::set_texture_2d(
	const Ren3dGlTexture2dPtr texture_2d) noexcept
{
	texture_2d_ = texture_2d;
}

Ren3dGlVertexInputPtr Ren3dGlContextImpl::get_vertex_input() const noexcept
{
	return vertex_input_;
}

void Ren3dGlContextImpl::set_vertex_input(
	const Ren3dGlVertexInputPtr vertex_input) noexcept
{
	vertex_input_ = vertex_input;
}

Ren3dGlShaderStagePtr Ren3dGlContextImpl::get_shader_stage() const noexcept
{
	return shader_stage_;
}

void Ren3dGlContextImpl::set_shader_stage(
	const Ren3dGlShaderStagePtr shader_stage) noexcept
{
	shader_stage_ = shader_stage;
}

void Ren3dGlContextImpl::clear(
	const Rgba8& rgba_8)
{
	if (clear_color_ != rgba_8)
	{
		clear_color_ = rgba_8;
		set_clear_color();
	}

	clear();
}

void Ren3dGlContextImpl::set_viewport(
	const Ren3dViewport& viewport)
try
{
	if (viewport_.x != viewport.x ||
		viewport_.y != viewport.y ||
		viewport_.width_ != viewport.width_ ||
		viewport_.height_ != viewport.height_)
	{
		viewport_.x = viewport.x;
		viewport_.y = viewport.y;
		viewport_.width_ = viewport.width_;
		viewport_.height_ = viewport.height_;
		set_viewport_rect();
	}

	if (viewport_.min_depth_ != viewport.min_depth_ ||
		viewport_.max_depth_ != viewport.max_depth_)
	{
		viewport_.min_depth_ = viewport.min_depth_;
		viewport_.max_depth_ = viewport.max_depth_;
		set_viewport_depth_range();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_scissor(
	const bool is_enable)
try
{
	if (is_scissor_enabled_ != is_enable)
	{
		is_scissor_enabled_ = is_enable;
		enable_scissor();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_scissor_box(
	const Ren3dScissorBox& scissor_box)
try
{
	if (scissor_box_.x != scissor_box.x ||
		scissor_box_.y != scissor_box.y ||
		scissor_box_.width_ != scissor_box.width_ ||
		scissor_box_.height_ != scissor_box.height_)
	{
		scissor_box_ = scissor_box;
		set_scissor_box();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_culling(
	const bool is_enable)
try
{
	if (is_culling_enabled_ != is_enable)
	{
		is_culling_enabled_ = is_enable;
		enable_culling();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_depth_test(
	const bool is_enable)
try
{
	if (is_depth_test_enabled_ != is_enable)
	{
		is_depth_test_enabled_ = is_enable;
		enable_depth_test();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_depth_write(
	const bool is_enable)
try
{
	if (is_depth_write_enabled_ != is_enable)
	{
		is_depth_write_enabled_ = is_enable;
		enable_depth_write();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_blending(
	const bool is_enable)
try
{
	if (is_blending_enabled_ != is_enable)
	{
		is_blending_enabled_ = is_enable;
		enable_blending();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_blending_func(
	const Ren3dBlendingFunc& blending_func)
try
{
	if (blending_func_.src_factor_ != blending_func.src_factor_ ||
		blending_func_.dst_factor_ != blending_func.dst_factor_)
	{
		blending_func_ = blending_func;
		set_blending_func();
	}
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_clear_color()
try
{
	glClearColor(
		static_cast<float>(clear_color_.r_) / 255.0F,
		static_cast<float>(clear_color_.g_) / 255.0F,
		static_cast<float>(clear_color_.b_) / 255.0F,
		static_cast<float>(clear_color_.a_) / 255.0F
	);

	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::clear()
try
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	Ren3dGlError::ensure_debug();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_clear_defaults()
try
{
	clear_color_ = {};
	set_clear_color();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_viewport_rect()
try
{
	Ren3dGlUtils::set_viewport_rect(viewport_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_viewport_depth_range()
try
{
	Ren3dGlUtils::set_viewport_depth_range(viewport_, gl_device_features_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_viewport_defaults()
try
{
	viewport_.x = 0;
	viewport_.y = 0;
	viewport_.width_ = 0;
	viewport_.height_ = 0;
	set_viewport_rect();

	viewport_.min_depth_ = 0.0F;
	viewport_.max_depth_ = 1.0F;
	set_viewport_depth_range();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_scissor()
try
{
	Ren3dGlUtils::enable_scissor(is_scissor_enabled_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_scissor_box()
try
{
	Ren3dGlUtils::set_scissor_box(scissor_box_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_scissor_defaults()
try
{
	is_scissor_enabled_ = false;
	enable_scissor();

	scissor_box_.x = 0;
	scissor_box_.y = 0;
	scissor_box_.width_ = 0;
	scissor_box_.height_ = 0;
	set_scissor_box();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_culling()
try
{
	Ren3dGlUtils::enable_culling(is_culling_enabled_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_culling_face()
try
{
	Ren3dGlUtils::set_culling_face(culling_face_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_culling_mode()
try
{
	Ren3dGlUtils::set_culling_mode(culling_mode_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_culling_defaults()
try
{
	is_culling_enabled_ = false;
	enable_culling();

	culling_face_ = Ren3dCullingFace::counter_clockwise;
	set_culling_face();

	culling_mode_ = Ren3dCullingMode::back;
	set_culling_mode();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_depth_test()
try
{
	Ren3dGlUtils::enable_depth_test(is_depth_test_enabled_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_depth_write()
try
{
	Ren3dGlUtils::enable_depth_write(is_depth_write_enabled_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_depth_defaults()
try
{
	is_depth_test_enabled_ = false;
	enable_depth_test();

	is_depth_write_enabled_ = false;
	enable_depth_write();
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::enable_blending()
try
{
	Ren3dGlUtils::enable_blending(is_blending_enabled_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_blending_func()
try
{
	Ren3dGlUtils::set_blending_func(blending_func_);
}
catch (...)
{
	fail_nested(__func__);
}

void Ren3dGlContextImpl::set_blending_defaults()
try
{
	is_blending_enabled_ = false;
	enable_blending();

	blending_func_.src_factor_ = Ren3dBlendingFactor::src_alpha;
	blending_func_.dst_factor_ = Ren3dBlendingFactor::one_minus_src_alpha;
	set_blending_func();
}
catch (...)
{
	fail_nested(__func__);
}

//
// Ren3dGlContextImpl
// ==========================================================================


// =========================================================================
// Ren3dGlContextFactory
//

class Ren3dGlContextFactoryException :
	public Exception
{
public:
	explicit Ren3dGlContextFactoryException(
		const char* message) noexcept
		:
		Exception{"REN_3D_GL_CTX_FACTORY", message}
	{
	}
}; // Ren3dGlContextFactoryException


class Ren3dGlContextFactory::Detail
{
public:
	[[noreturn]]
	static void fail(
		const char* message)
	{
		throw Ren3dGlContextFactoryException{message};
	}

	[[noreturn]]
	static void fail_nested(
		const char* message)
	{
		std::throw_with_nested(Ren3dGlContextFactoryException{message});
	}
}; // Ren3dGlContextFactory::Detail

Ren3dGlContextUPtr Ren3dGlContextFactory::create(
	const Ren3dKind renderer_kind,
	const Ren3dDeviceFeatures& device_features,
	const Ren3dGlDeviceFeatures& gl_device_features)
try
{
	switch (renderer_kind)
	{
		case Ren3dKind::gl_2_0:
		case Ren3dKind::gl_3_2_core:
		case Ren3dKind::gles_2_0:
			return std::make_unique<Ren3dGlContextImpl>(device_features, gl_device_features);

		default:
			Detail::fail("Unsupported renderer kind.");
	}
}
catch (...)
{
	Detail::fail_nested(__func__);
}

//
// Ren3dGlContextFactory
// =========================================================================


} // detail
} // bstone
