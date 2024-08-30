/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Context

#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"

#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_buffer.h"
#include "bstone_gl_r3r_context.h"
#include "bstone_gl_r3r_device_features.h"
#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_r2_texture.h"
#include "bstone_gl_r3r_sampler.h"
#include "bstone_gl_r3r_sampler_mgr.h"
#include "bstone_gl_r3r_shader.h"
#include "bstone_gl_r3r_shader_stage.h"
#include "bstone_gl_r3r_utils.h"
#include "bstone_gl_r3r_vertex_input.h"
#include "bstone_gl_r3r_vertex_input_mgr.h"

namespace bstone {

GlR3rContext::GlR3rContext() noexcept = default;

GlR3rContext::~GlR3rContext() = default;

// ==========================================================================

namespace {

class GlR3rContextImpl final : public GlR3rContext
{
public:
	GlR3rContextImpl(
		const R3rDeviceFeatures& device_features,
		const GlR3rDeviceFeatures& gl_device_features);

	GlR3rContextImpl(const GlR3rContextImpl& rhs) = delete;
	~GlR3rContextImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	const R3rDeviceFeatures& get_device_features() const noexcept override;
	const GlR3rDeviceFeatures& get_gl_device_features() const noexcept override;

	GlR3rBufferUPtr create_buffer(const R3rBufferInitParam& param) override;

	GlR3rSamplerMgr& get_sampler_manager() const noexcept override;
	GlR3rVertexInputMgr& get_vertex_input_manager() const noexcept override;

	GlR3rShaderUPtr create_shader(const R3rShaderInitParam& param) override;

	GlR3rSampler* get_sampler() const noexcept override;
	void set_sampler(GlR3rSampler* sampler) noexcept override;

	R3rR2TextureUPtr create_r2_texture(const R3rR2TextureInitParam& param) override;
	void bind_r2_texture(GlR3rR2Texture* r2_texture) override;
	GlR3rR2Texture* get_r2_texture() const noexcept override;
	void set_r2_texture(GlR3rR2Texture* r2_texture) noexcept override;

	GlR3rVertexInput* get_vertex_input() const noexcept override;
	void set_vertex_input(GlR3rVertexInput* vertex_input) noexcept override;

	R3rShaderStageUPtr create_shader_stage(const R3rShaderStageInitParam& param) override;
	void bind_shader_stage(R3rShaderStage* shader_stage) override;
	GlR3rShaderStage* get_shader_stage() const noexcept override;
	void set_shader_stage(GlR3rShaderStage* shader_stage) noexcept override;

	void clear(const sys::Color& color) override;

	void set_viewport(const R3rViewport& viewport) override;
	void enable_scissor(bool is_enable) override;
	void set_scissor_box(const R3rScissorBox& scissor_box) override;

	void enable_culling(bool is_enable) override;
	void enable_depth_test(bool is_enable) override;
	void enable_depth_write(bool is_enable) override;

	void enable_blending(bool is_enable) override;
	void set_blending_func(const R3rBlendingFunc& func) override;

private:
	const R3rDeviceFeatures& device_features_;
	const GlR3rDeviceFeatures& gl_device_features_;

	sys::Color clear_color_{};

	R3rViewport viewport_{};
	bool is_scissor_enabled_{};
	R3rScissorBox scissor_box_{};

	bool is_culling_enabled_{};
	R3rCullingFace culling_face_{};
	R3rCullingMode culling_mode_{};

	bool is_depth_test_enabled_{};
	bool is_depth_write_enabled_{};

	bool is_blending_enabled_{};
	R3rBlendingFunc blending_func_{};

	GlR3rVertexInputMgrUPtr vertex_input_manager_{};
	GlR3rSamplerMgrUPtr sampler_manager_{};

	GlR3rSampler* sampler_{};
	GlR3rR2Texture* r2_texture_{};
	GlR3rVertexInput* vertex_input_{};
	GlR3rShaderStage* shader_stage_{};

private:
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
};

// ==========================================================================

using GlR3rContextImplPool = SinglePoolResource<GlR3rContextImpl>;
GlR3rContextImplPool gl_r3r_context_impl_pool{};

// ==========================================================================

GlR3rContextImpl::GlR3rContextImpl(
	const R3rDeviceFeatures& device_features,
	const GlR3rDeviceFeatures& gl_device_features)
try
	:
	device_features_{device_features},
	gl_device_features_{gl_device_features}
{
	vertex_input_manager_ = make_gl_r3r_vertex_input_mgr(*this);
	sampler_manager_ = make_gl_r3r_sampler_mgr(*this);

	set_max_mipmap_quality();

	set_clear_defaults();
	set_viewport_defaults();
	set_scissor_defaults();
	set_culling_defaults();
	set_depth_defaults();
	set_blending_defaults();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rContextImpl::~GlR3rContextImpl() = default;

void GlR3rContextImpl::set_max_mipmap_quality()
try {
	if (!device_features_.is_mipmap_available)
	{
		return;
	}

	if (gl_device_features_.context_profile == sys::GlContextProfile::core)
	{
		return;
	}

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void* GlR3rContextImpl::operator new(std::size_t size)
try {
	return gl_r3r_context_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::operator delete(void* ptr)
{
	gl_r3r_context_impl_pool.deallocate(ptr);
}

const R3rDeviceFeatures& GlR3rContextImpl::get_device_features() const noexcept
{
	return device_features_;
}

const GlR3rDeviceFeatures& GlR3rContextImpl::get_gl_device_features() const noexcept
{
	return gl_device_features_;
}

GlR3rBufferUPtr GlR3rContextImpl::create_buffer(const R3rBufferInitParam& param)
try {
	return make_gl_r3r_buffer(*this, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rSamplerMgr& GlR3rContextImpl::get_sampler_manager() const noexcept
{
	return *sampler_manager_;
}

GlR3rVertexInputMgr& GlR3rContextImpl::get_vertex_input_manager() const noexcept
{
	return *vertex_input_manager_;
}

GlR3rShaderUPtr GlR3rContextImpl::create_shader(const R3rShaderInitParam& param)
try {
	return make_gl_r3r_shader(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rSampler* GlR3rContextImpl::get_sampler() const noexcept
{
	return sampler_;
}

void GlR3rContextImpl::set_sampler(GlR3rSampler* sampler) noexcept
{
	sampler_ = sampler;
}

R3rR2TextureUPtr GlR3rContextImpl::create_r2_texture(const R3rR2TextureInitParam& param)
try {
	return make_gl_r3r_r2_texture(*this, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::bind_r2_texture(GlR3rR2Texture* r2_texture)
try {
	if (r2_texture != nullptr)
	{
		r2_texture->set();
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rR2Texture* GlR3rContextImpl::get_r2_texture() const noexcept
{
	return r2_texture_;
}

void GlR3rContextImpl::set_r2_texture(GlR3rR2Texture* r2_texture) noexcept
{
	r2_texture_ = r2_texture;
}

GlR3rVertexInput* GlR3rContextImpl::get_vertex_input() const noexcept
{
	return vertex_input_;
}

void GlR3rContextImpl::set_vertex_input(GlR3rVertexInput* vertex_input) noexcept
{
	vertex_input_ = vertex_input;
}

R3rShaderStageUPtr GlR3rContextImpl::create_shader_stage(const R3rShaderStageInitParam& param)
try {
	return make_gl_r3r_shader_stage(*this, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::bind_shader_stage(R3rShaderStage* shader_stage)
try {
	if (shader_stage != nullptr)
	{
		static_cast<GlR3rShaderStage*>(shader_stage)->set();
	}
	else
	{
		glUseProgram(0);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rShaderStage* GlR3rContextImpl::get_shader_stage() const noexcept
{
	return shader_stage_;
}

void GlR3rContextImpl::set_shader_stage(GlR3rShaderStage* shader_stage) noexcept
{
	shader_stage_ = shader_stage;
}

void GlR3rContextImpl::clear(const sys::Color& color)
{
	if (clear_color_ != color)
	{
		clear_color_ = color;
		set_clear_color();
	}

	clear();
}

void GlR3rContextImpl::set_viewport(const R3rViewport& viewport)
try {
	if (viewport_.x != viewport.x ||
		viewport_.y != viewport.y ||
		viewport_.width != viewport.width ||
		viewport_.height != viewport.height)
	{
		viewport_.x = viewport.x;
		viewport_.y = viewport.y;
		viewport_.width = viewport.width;
		viewport_.height = viewport.height;
		set_viewport_rect();
	}

	if (viewport_.min_depth != viewport.min_depth ||
		viewport_.max_depth != viewport.max_depth)
	{
		viewport_.min_depth = viewport.min_depth;
		viewport_.max_depth = viewport.max_depth;
		set_viewport_depth_range();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_scissor(bool is_enable)
try {
	if (is_scissor_enabled_ != is_enable)
	{
		is_scissor_enabled_ = is_enable;
		enable_scissor();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_scissor_box(const R3rScissorBox& scissor_box)
try {
	if (scissor_box_.x != scissor_box.x ||
		scissor_box_.y != scissor_box.y ||
		scissor_box_.width != scissor_box.width ||
		scissor_box_.height != scissor_box.height)
	{
		scissor_box_ = scissor_box;
		set_scissor_box();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_culling(bool is_enable)
try {
	if (is_culling_enabled_ != is_enable)
	{
		is_culling_enabled_ = is_enable;
		enable_culling();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_depth_test(bool is_enable)
try {
	if (is_depth_test_enabled_ != is_enable)
	{
		is_depth_test_enabled_ = is_enable;
		enable_depth_test();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_depth_write(bool is_enable)
try {
	if (is_depth_write_enabled_ != is_enable)
	{
		is_depth_write_enabled_ = is_enable;
		enable_depth_write();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_blending(bool is_enable)
try {
	if (is_blending_enabled_ != is_enable)
	{
		is_blending_enabled_ = is_enable;
		enable_blending();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_blending_func(const R3rBlendingFunc& blending_func)
try {
	if (blending_func_.src_factor != blending_func.src_factor ||
		blending_func_.dst_factor != blending_func.dst_factor)
	{
		blending_func_ = blending_func;
		set_blending_func();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_clear_color()
try {
	glClearColor(
		static_cast<float>(clear_color_.r) / 255.0F,
		static_cast<float>(clear_color_.g) / 255.0F,
		static_cast<float>(clear_color_.b) / 255.0F,
		static_cast<float>(clear_color_.a) / 255.0F);

	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::clear()
try {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_clear_defaults()
try {
	clear_color_ = sys::Color{};
	set_clear_color();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_viewport_rect()
try {
	GlR3rUtils::set_viewport_rect(viewport_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_viewport_depth_range()
try {
	GlR3rUtils::set_viewport_depth_range(viewport_, gl_device_features_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_viewport_defaults()
try {
	viewport_.x = 0;
	viewport_.y = 0;
	viewport_.width = 0;
	viewport_.height = 0;
	set_viewport_rect();

	viewport_.min_depth = 0.0F;
	viewport_.max_depth = 1.0F;
	set_viewport_depth_range();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_scissor()
try {
	GlR3rUtils::enable_scissor(is_scissor_enabled_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_scissor_box()
try {
	GlR3rUtils::set_scissor_box(scissor_box_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_scissor_defaults()
try {
	is_scissor_enabled_ = false;
	enable_scissor();

	scissor_box_.x = 0;
	scissor_box_.y = 0;
	scissor_box_.width = 0;
	scissor_box_.height = 0;
	set_scissor_box();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_culling()
try {
	GlR3rUtils::enable_culling(is_culling_enabled_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_culling_face()
try {
	GlR3rUtils::set_culling_face(culling_face_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_culling_mode()
try {
	GlR3rUtils::set_culling_mode(culling_mode_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_culling_defaults()
try {
	is_culling_enabled_ = false;
	enable_culling();

	culling_face_ = R3rCullingFace::counter_clockwise;
	set_culling_face();

	culling_mode_ = R3rCullingMode::back;
	set_culling_mode();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_depth_test()
try {
	GlR3rUtils::enable_depth_test(is_depth_test_enabled_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_depth_write()
try {
	GlR3rUtils::enable_depth_write(is_depth_write_enabled_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_depth_defaults()
try {
	is_depth_test_enabled_ = false;
	enable_depth_test();

	is_depth_write_enabled_ = false;
	enable_depth_write();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::enable_blending()
try {
	GlR3rUtils::enable_blending(is_blending_enabled_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_blending_func()
try {
	GlR3rUtils::set_blending_func(blending_func_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rContextImpl::set_blending_defaults()
try {
	is_blending_enabled_ = false;
	enable_blending();

	blending_func_.src_factor = R3rBlendingFactor::src_alpha;
	blending_func_.dst_factor = R3rBlendingFactor::one_minus_src_alpha;
	set_blending_func();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// =========================================================================

} // namespace

GlR3rContextUPtr make_gl_r3r_context(
	R3rType renderer_type,
	const R3rDeviceFeatures& device_features,
	const GlR3rDeviceFeatures& gl_device_features)
try {
	switch (renderer_type)
	{
		case R3rType::gl_2_0:
		case R3rType::gl_3_2_core:
		case R3rType::gles_2_0:
			return std::make_unique<GlR3rContextImpl>(device_features, gl_device_features);

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported renderer type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
