/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer

#include <list>

#include "bstone_algorithm.h"
#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_unique_resource.h"

#include "bstone_sys_gl_context.h"
#include "bstone_sys_gl_current_context.h"
#include "bstone_sys_swap_interval_type.h"
#include "bstone_sys_video_mgr.h"
#include "bstone_sys_window_mgr.h"

#include "bstone_r3r_cmd_buffer.h"
#include "bstone_r3r_limits.h"
#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r.h"
#include "bstone_gl_r3r_buffer.h"
#include "bstone_gl_r3r_context.h"
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

namespace {

class GlR3rImpl final : public R3r
{
public:
	GlR3rImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param);
	~GlR3rImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	R3rType do_get_type() const noexcept override;
	StringView do_get_name() const noexcept override;
	StringView do_get_description() const noexcept override;

	const R3rDeviceFeatures& do_get_device_features() const noexcept override;
	const R3rDeviceInfo& do_get_device_info() const noexcept override;

	void do_enable_checking_api_calls_for_errors(bool is_enable) override;

	sys::Window& do_get_window() const noexcept override;
	void do_handle_resize(sys::WindowSize new_size) override;

	bool do_get_vsync() const noexcept override;
	void do_enable_vsync(bool is_enabled) override;

	void do_set_anti_aliasing(R3rAaType aa_type, int aa_value) override;

	void do_read_pixels(
		sys::PixelFormat pixel_format,
		void* buffer,
		bool& is_flipped_vertically) override;

	void do_present() override;

	R3rBufferUPtr do_create_buffer(const R3rBufferInitParam& param) override;
	R3rR2TextureUPtr do_create_r2_texture(const R3rR2TextureInitParam& param) override;
	R3rSamplerUPtr do_create_sampler(const R3rSamplerInitParam& param) override;
	R3rVertexInputUPtr do_create_vertex_input(const R3rCreateVertexInputParam& param) override;
	R3rShaderUPtr do_create_shader(const R3rShaderInitParam& param) override;
	R3rShaderStageUPtr do_create_shader_stage(const R3rShaderStageInitParam& param) override;
	void do_submit_commands(Span<R3rCmdBuffer*> command_buffers) override;

private:
	class FboDeleter
	{
	public:
		FboDeleter();
		explicit FboDeleter(PFNGLDELETEFRAMEBUFFERSPROC gl_func) noexcept;
		void operator()(GLuint gl_name) noexcept;

	private:
		PFNGLDELETEFRAMEBUFFERSPROC gl_func_{};
	};

	using FboResource = UniqueResource<GLuint, FboDeleter>;

	class RboDeleter
	{
	public:
		RboDeleter() noexcept;
		explicit RboDeleter(PFNGLDELETERENDERBUFFERSPROC gl_func) noexcept;
		void operator()(GLuint gl_name) noexcept;

	private:
		PFNGLDELETERENDERBUFFERSPROC gl_func_{};
	};

	using RboResource = UniqueResource<GLuint, RboDeleter>;

	using Shaders = std::list<GlR3rShaderUPtr>;
	using ShaderStages = std::list<GlR3rShaderStageUPtr>;

private:
	sys::VideoMgr& video_mgr_;
	sys::WindowMgr& window_mgr_;

	R3rType type_{};
	StringView name_{};
	StringView description_{};

	std::string device_name_{};
	std::string device_vendor_{};
	std::string device_version_{};
	R3rDeviceInfo device_info_{};
	R3rDeviceFeatures device_features_{};
	GlR3rDeviceFeatures gl_device_features_{};

	int screen_width_{};
	int screen_height_{};

	R3rAaType aa_type_{};
	int aa_value_{};

	sys::GlCurrentContext& gl_current_context_;
	sys::WindowUPtr window_{};
	sys::GlContextUPtr gl_context_{};

	GlR3rExtensionMgrUPtr extension_manager_{};
	GlR3rContextUPtr context_{};

	FboResource msaa_fbo_{0, FboDeleter{}};
	RboResource msaa_color_rb_{0, RboDeleter{}};
	RboResource msaa_depth_rb_{0, RboDeleter{}};

	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer_;
	PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer_;
	PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer_;
	PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus_;
	PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers_;
	PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers_;
	PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer_;
	PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers_;
	PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers_;
	PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage_;
	PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample_;

private:
	void set_device_info();
	void set_name_and_description();

	RboResource create_renderbuffer();
	RboResource create_renderbuffer(int width, int height, int sample_count, GLenum gl_internal_format);
	void bind_renderbuffer(GLuint gl_name);

	FboResource create_framebuffer();
	void bind_framebuffer(GLenum gl_target, GLuint gl_name);

	void blit_framebuffer(
		int src_width,
		int src_height,
		int dst_width,
		int dst_height,
		bool is_linear_filter);

	void destroy_msaa_color_rb() noexcept;
	void destroy_msaa_depth_rb() noexcept;
	void destroy_msaa_fbo() noexcept;
	void destroy_msaa_framebuffer() noexcept;
	void create_msaa_color_rb(int width, int height, int sample_count);
	void create_msaa_depth_rb(int width, int height, int sample_count);
	void create_msaa_framebuffer();

	void destroy_framebuffers() noexcept;
	void initialize_framebuffer_funcs() noexcept;
	void create_framebuffers();
	void blit_framebuffers();
	void bind_framebuffers();
	void bind_framebuffers_for_read_pixels();

	void disable_aa();
	void set_msaa(int aa_value);

	void submit_clear(const R3rClearCmd& command);

	void submit_culling(const R3rEnableCullingCmd& command);

	void submit_enable_depth_test(const R3rEnableDepthTestCmd& command);
	void submit_enable_depth_write(const R3rEnableDepthWriteCmd& command);

	void submit_enable_blending(const R3rEnableBlendingCmd& command);
	void submit_set_blending_func(const R3rSetBlendingFuncCmd& command);

	void submit_set_viewport(const R3rSetViewportCmd& command);
	void submit_enable_scissor(const R3rEnableScissorCmd& command);
	void submit_set_scissor_box(const R3rSetScissorBoxCmd& command);

	void submit_set_texture(const R3rSetTextureCmd& command);
	void submit_set_sampler(const R3rSetSamplerCmd& command);
	void submit_set_vertex_input(const R3rSetVertexInputCmd& command);
	void submit_set_shader_stage(const R3rSetShaderStageCmd& command);
	void submit_set_int32_uniform(const R3rSetInt32UniformCmd& command);
	void submit_set_float32_uniform(const R3rSetFloat32UniformCmd& command);
	void submit_set_vec2_uniform(const R3rSetVec2UniformCmd& command);
	void submit_set_vec4_uniform(const R3rSetVec4UniformCmd& command);
	void submit_set_mat4_uniform(const R3rSetMat4UniformCmd& command);
	void submit_set_sampler_2d_uniform(const R3rSetR2SamplerUniformCmd& command);

	void submit_draw_indexed(const R3rDrawIndexedCmd& command);
};

// ==========================================================================

using GlR3rImplPool = SinglePoolResource<GlR3rImpl>;
GlR3rImplPool gl_r3r_impl_pool{};

// ==========================================================================

GlR3rImpl::~GlR3rImpl() = default;

GlR3rImpl::GlR3rImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param)
try
	:
	video_mgr_{video_mgr},
	window_mgr_{window_mgr},
	gl_current_context_{video_mgr.get_gl_current_context()}
{
	switch (param.renderer_type)
	{
		case R3rType::gl_2_0:
		case R3rType::gl_3_2_core:
		case R3rType::gles_2_0:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported renderer type.");
	}

	type_ = param.renderer_type;

	GlR3rUtils::probe_msaa(
		type_,
		gl_current_context_.get_symbol_resolver(),
		window_mgr,
		device_features_,
		gl_device_features_);

	aa_type_ = param.aa_type;
	aa_value_ = param.aa_value;

	auto window_param = R3rUtilsCreateWindowParam{};
	window_param.renderer_type = type_;
	window_param.aa_type = aa_type_;
	window_param.aa_value = aa_value_;

	if (window_param.aa_type == R3rAaType::ms)
	{
		if (device_features_.is_msaa_available)
		{
			if (aa_value_ <= 0)
			{
				aa_value_ = device_features_.max_msaa_degree;
			}

			if (aa_value_ < R3rLimits::min_aa_off())
			{
				aa_value_ = R3rLimits::min_aa_off();
			}

			if (aa_value_ > device_features_.max_msaa_degree)
			{
				aa_value_ = device_features_.max_msaa_degree;
			}

			if (device_features_.is_msaa_render_to_window)
			{
				window_param.aa_value = aa_value_;
			}
			else
			{
				window_param.aa_type = R3rAaType::none;
				window_param.aa_value = 0;
				window_param.is_default_depth_buffer_disabled = true;
			}
		}
		else
		{
			window_param.aa_type = R3rAaType::none;
			window_param.aa_value = 0;
		}
	}

	GlR3rUtils::create_window_and_context(window_param, window_mgr, window_, gl_context_);

	const auto drawable_size = window_->gl_get_drawable_size();
	screen_width_ = drawable_size.width;
	screen_height_ = drawable_size.height;

	if (aa_type_ == R3rAaType::ms && device_features_.is_msaa_render_to_window)
	{
		aa_value_ = GlR3rUtils::get_window_msaa_value(gl_context_->get_attributes());
	}

	extension_manager_ = make_gl_r3r_extension_mgr(gl_current_context_.get_symbol_resolver());

	if (extension_manager_ == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create an extension manager.");
	}

	switch (type_)
	{
		case R3rType::gl_2_0:
			extension_manager_->probe(GlR3rExtensionId::v2_0);

			if (!extension_manager_->has(GlR3rExtensionId::v2_0))
			{
				BSTONE_THROW_STATIC_SOURCE("Failed to load OpenGL 2.0 symbols.");
			}

			break;

		case R3rType::gl_3_2_core:
			extension_manager_->probe(GlR3rExtensionId::v3_2_core);

			if (!extension_manager_->has(GlR3rExtensionId::v3_2_core))
			{
				BSTONE_THROW_STATIC_SOURCE("Failed to load OpenGL 3.2 core symbols.");
			}

			break;

		case R3rType::gles_2_0:
			extension_manager_->probe(GlR3rExtensionId::es_v2_0);

			if (!extension_manager_->has(GlR3rExtensionId::es_v2_0))
			{
				BSTONE_THROW_STATIC_SOURCE("Failed to load OpenGL ES 2.0 symbols.");
			}

			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported renderer type.");
	}

	GlR3rUtils::set_renderer_features(device_features_);

	gl_device_features_.context_profile = gl_context_->get_attributes().profile;

	GlR3rUtils::probe_anisotropy(extension_manager_.get(), device_features_);
	GlR3rUtils::probe_npot(extension_manager_.get(), device_features_);
	GlR3rUtils::probe_mipmap(extension_manager_.get(), device_features_, gl_device_features_);
	GlR3rUtils::probe_framebuffer(extension_manager_.get(), gl_device_features_ );
	GlR3rUtils::probe_sampler(extension_manager_.get(), device_features_);
	GlR3rUtils::probe_max_vertex_arrays(device_features_);
	GlR3rUtils::probe_buffer_storage(extension_manager_.get(), gl_device_features_);
	GlR3rUtils::probe_dsa(extension_manager_.get(), gl_device_features_);
	GlR3rUtils::probe_sso(extension_manager_.get(), gl_device_features_ );

	if (device_features_.max_vertex_input_locations <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("No vertex input locations.");
	}

	GlR3rUtils::probe_vsync(gl_current_context_, device_features_);
	GlR3rUtils::probe_vao(extension_manager_.get(), gl_device_features_);

	context_ = make_gl_r3r_context(type_, device_features_, gl_device_features_ );

	if (device_features_.is_vsync_available)
	{
		gl_current_context_.set_swap_interval(
			param.is_vsync ? sys::SwapIntervalType::standard : sys::SwapIntervalType::none);
	}

	create_framebuffers();

	set_device_info();
	set_name_and_description();

	// Present.
	//
	context_->clear(sys::Color{});
	present();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void* GlR3rImpl::operator new(std::size_t size)
try {
	return gl_r3r_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::operator delete(void* ptr)
{
	gl_r3r_impl_pool.deallocate(ptr);
}

R3rType GlR3rImpl::do_get_type() const noexcept
{
	return type_;
}

StringView GlR3rImpl::do_get_name() const noexcept
{
	return name_;
}

StringView GlR3rImpl::do_get_description() const noexcept
{
	return description_;
}

const R3rDeviceFeatures& GlR3rImpl::do_get_device_features() const noexcept
{
	return device_features_;
}

const R3rDeviceInfo& GlR3rImpl::do_get_device_info() const noexcept
{
	return device_info_;
}

void GlR3rImpl::do_enable_checking_api_calls_for_errors(bool is_enable)
{
	GlR3rError::enable_checking(is_enable);
}

sys::Window& GlR3rImpl::do_get_window() const noexcept
{
	return *window_;
}

void GlR3rImpl::do_handle_resize(sys::WindowSize new_size)
try {
	const auto size_changed = screen_width_ != new_size.width || screen_height_ != new_size.height;

	screen_width_ = new_size.width;
	screen_height_ = new_size.height;

	if (size_changed && gl_device_features_.is_framebuffer_available)
	{
		destroy_msaa_framebuffer();
		create_msaa_framebuffer();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool GlR3rImpl::do_get_vsync() const noexcept
{
	if (!device_features_.is_vsync_available)
	{
		return false;
	}

	return gl_current_context_.get_swap_interval() == sys::SwapIntervalType::standard;
}

void GlR3rImpl::do_enable_vsync(bool is_enabled)
try {
	if (!device_features_.is_vsync_available)
	{
		BSTONE_THROW_STATIC_SOURCE("Not available.");
	}

	if (device_features_.is_vsync_requires_restart)
	{
		BSTONE_THROW_STATIC_SOURCE("Requires restart.");
	}

	gl_current_context_.set_swap_interval(
		is_enabled ? sys::SwapIntervalType::standard : sys::SwapIntervalType::none);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::do_set_anti_aliasing(R3rAaType aa_type, int aa_value)
try {
	switch (aa_type)
	{
		case R3rAaType::none:
		case R3rAaType::ms:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid anti-aliasing type.");
	}

	auto clamped_aa_value = aa_value;

	if (clamped_aa_value < R3rLimits::min_aa_off())
	{
		clamped_aa_value = R3rLimits::min_aa_off();
	}

	if (clamped_aa_value > R3rLimits::max_aa())
	{
		clamped_aa_value = R3rLimits::max_aa();
	}

	switch (aa_type)
	{
		case R3rAaType::none:
			disable_aa();
			return;

		case R3rAaType::ms:
			set_msaa(clamped_aa_value);
			return;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid anti-aliasing type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::do_read_pixels(
	sys::PixelFormat pixel_format,
	void* buffer,
	bool& is_flipped_vertically)
try {
	BSTONE_ASSERT(buffer != nullptr);

	switch (pixel_format)
	{
		case sys::PixelFormat::r8g8b8:
			break;

		default: BSTONE_THROW_STATIC_SOURCE("Unsupported pixel format.");
	}

	is_flipped_vertically = true;
	bind_framebuffers_for_read_pixels();

	glReadBuffer(GL_BACK);
	GlR3rError::ensure_no_errors();
  
	glReadPixels(0, 0, screen_width_, screen_height_, GL_RGB, GL_UNSIGNED_BYTE, buffer);
	GlR3rError::ensure_no_errors();

	bind_framebuffers();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::do_present()
try {
	blit_framebuffers();
	GlR3rError::ensure_no_errors();

	window_->gl_swap_buffers();
	bind_framebuffers();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rBufferUPtr GlR3rImpl::do_create_buffer(const R3rBufferInitParam& param)
try {
	return context_->create_buffer(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rVertexInputUPtr GlR3rImpl::do_create_vertex_input(const R3rCreateVertexInputParam& param)
try {
	return context_->get_vertex_input_manager().create(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderUPtr GlR3rImpl::do_create_shader(const R3rShaderInitParam& param)
try {
	return context_->create_shader(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderStageUPtr GlR3rImpl::do_create_shader_stage(const R3rShaderStageInitParam& param)
try {
	return context_->create_shader_stage(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rR2TextureUPtr GlR3rImpl::do_create_r2_texture(const R3rR2TextureInitParam& param)
try {
	return context_->create_r2_texture(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSamplerUPtr GlR3rImpl::do_create_sampler(const R3rSamplerInitParam& param)
try {
	return context_->get_sampler_manager().create(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::do_submit_commands(Span<R3rCmdBuffer*> command_buffers)
try {
	for (auto command_buffer : command_buffers)
	{
		if (command_buffer == nullptr)
		{
			BSTONE_THROW_STATIC_SOURCE("Null command buffer.");
		}

		if (!command_buffer->is_enabled())
		{
			continue;
		}

		const auto command_count = command_buffer->get_count();

		command_buffer->begin_read();

		for (int j = 0; j < command_count; ++j)
		{
			const auto command_id = command_buffer->read_command_id();

			switch (command_id)
			{
			case R3rCmdId::clear:
				submit_clear(command_buffer->read_clear());
				break;

			case R3rCmdId::enable_culling:
				submit_culling(command_buffer->read_enable_culling());
				break;

			case R3rCmdId::enable_depth_test:
				submit_enable_depth_test(command_buffer->read_enable_depth_test());
				break;

			case R3rCmdId::enable_depth_write:
				submit_enable_depth_write(command_buffer->read_enable_depth_write());
				break;

			case R3rCmdId::set_viewport:
				submit_set_viewport(command_buffer->read_set_viewport());
				break;

			case R3rCmdId::enable_scissor:
				submit_enable_scissor(command_buffer->read_enable_scissor());
				break;

			case R3rCmdId::set_scissor_box:
				submit_set_scissor_box(command_buffer->read_set_scissor_box());
				break;

			case R3rCmdId::enable_blending:
				submit_enable_blending(command_buffer->read_enable_blending());
				break;

			case R3rCmdId::set_blending_func:
				submit_set_blending_func(command_buffer->read_set_blending_func());
				break;

			case R3rCmdId::set_texture:
				submit_set_texture(command_buffer->read_set_texture());
				break;

			case R3rCmdId::set_sampler:
				submit_set_sampler(command_buffer->read_set_sampler());
				break;

			case R3rCmdId::set_vertex_input:
				submit_set_vertex_input(command_buffer->read_set_vertex_input());
				break;

			case R3rCmdId::set_shader_stage:
				submit_set_shader_stage(command_buffer->read_set_shader_stage());
				break;

			case R3rCmdId::set_int32_uniform:
				submit_set_int32_uniform(command_buffer->read_set_int32_uniform());
				break;

			case R3rCmdId::set_float32_uniform:
				submit_set_float32_uniform(command_buffer->read_set_float32_uniform());
				break;

			case R3rCmdId::set_vec2_uniform:
				submit_set_vec2_uniform(command_buffer->read_set_vec2_uniform());
				break;

			case R3rCmdId::set_vec4_uniform:
				submit_set_vec4_uniform(command_buffer->read_set_vec4_uniform());
				break;

			case R3rCmdId::set_mat4_uniform:
				submit_set_mat4_uniform(command_buffer->read_set_mat4_uniform());
				break;

			case R3rCmdId::set_r2_sampler_uniform:
				submit_set_sampler_2d_uniform(command_buffer->read_set_r2_sampler_uniform());
				break;

			case R3rCmdId::draw_indexed:
				submit_draw_indexed(command_buffer->read_draw_indexed());
				break;

			default:
				BSTONE_THROW_STATIC_SOURCE("Unsupported command id.");
			}
		}

		command_buffer->end_read();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rImpl::FboDeleter::FboDeleter() = default;

GlR3rImpl::FboDeleter::FboDeleter(PFNGLDELETEFRAMEBUFFERSPROC gl_func) noexcept
	:
	gl_func_{gl_func}
{}

void GlR3rImpl::FboDeleter::operator()(GLuint gl_name) noexcept
{
	BSTONE_ASSERT(gl_func_ != nullptr);
	gl_func_(1, &gl_name);
	GlR3rError::ensure_no_errors_assert();
}

GlR3rImpl::RboDeleter::RboDeleter() noexcept = default;

GlR3rImpl::RboDeleter::RboDeleter(PFNGLDELETERENDERBUFFERSPROC gl_func) noexcept
	:
	gl_func_{gl_func}
{}

void GlR3rImpl::RboDeleter::operator()(GLuint gl_name) noexcept
{
	BSTONE_ASSERT(gl_func_ != nullptr);
	gl_func_(1, &gl_name);
	GlR3rError::ensure_no_errors_assert();
}

void GlR3rImpl::set_device_info()
try {
	const auto device_info = GlR3rUtils::get_device_info();

	device_name_.assign(device_info.name.cbegin(), device_info.name.cend());
	device_vendor_.assign(device_info.vendor.cbegin(), device_info.vendor.cend());
	device_version_.assign(device_info.version.cbegin(), device_info.version.cend());

	device_info_.name = StringView{device_name_.data(), static_cast<std::intptr_t>(device_name_.size())};
	device_info_.vendor = StringView{device_vendor_.data(), static_cast<std::intptr_t>(device_vendor_.size())};
	device_info_.version = StringView{device_version_.data(), static_cast<std::intptr_t>(device_version_.size())};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::set_name_and_description()
try {
	switch (type_)
	{
		case R3rType::gl_2_0:
			name_ = "GL2.0";
			description_ = "OpenGL 2.0+";
			break;

		case R3rType::gl_3_2_core:
			name_ = "GL3.2C";
			description_ = "OpenGL 3.2 core";
			break;

		case R3rType::gles_2_0:
			name_ = "GLES2.0";
			description_ = "OpenGL ES 2.0";
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported renderer type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rImpl::RboResource GlR3rImpl::create_renderbuffer()
try {
	if (!gl_device_features_.is_framebuffer_available)
	{
		BSTONE_THROW_STATIC_SOURCE("Framebuffer not available.");
	}

	auto gl_name = GLuint{};
	glGenRenderbuffers_(1, &gl_name);
	GlR3rError::check_optionally();

	auto rbo_resource = RboResource{gl_name, RboDeleter{glDeleteRenderbuffers_}};

	if (rbo_resource.is_empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create OpenGL renderbuffer object.");
	}

	return rbo_resource;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::bind_renderbuffer(GLuint gl_renderbuffer_name)
try {
	glBindRenderbuffer_(GL_RENDERBUFFER, gl_renderbuffer_name);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rImpl::FboResource GlR3rImpl::create_framebuffer()
try {
	if (!gl_device_features_.is_framebuffer_available)
	{
		BSTONE_THROW_STATIC_SOURCE("Framebuffer not available.");
	}

	auto gl_name = GLuint{};
	glGenFramebuffers_(1, &gl_name);
	GlR3rError::check_optionally();

	auto fbo_resource = FboResource{gl_name, FboDeleter{glDeleteFramebuffers_}};

	if (fbo_resource.is_empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create OpenGL framebuffer object.");
	}

	return fbo_resource;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::bind_framebuffer(GLenum gl_target, GLuint gl_name)
try {
	BSTONE_ASSERT(gl_device_features_.is_framebuffer_available);

	glBindFramebuffer_(gl_target, gl_name);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::blit_framebuffer(
	int src_width,
	int src_height,
	int dst_width,
	int dst_height,
	bool is_linear_filter)
try {
	BSTONE_ASSERT(src_width > 0);
	BSTONE_ASSERT(src_height > 0);
	BSTONE_ASSERT(dst_width > 0);
	BSTONE_ASSERT(dst_height > 0);

	BSTONE_ASSERT(gl_device_features_.is_framebuffer_available);

	const auto gl_filter = (is_linear_filter ? GL_LINEAR : GL_NEAREST);

	glBlitFramebuffer_(
		0,
		0,
		src_width,
		src_height,
		0,
		0,
		dst_width,
		dst_height,
		GL_COLOR_BUFFER_BIT,
		gl_filter);

	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rImpl::RboResource GlR3rImpl::create_renderbuffer(
	int width,
	int height,
	int sample_count,
	GLenum gl_internal_format)
try {
	BSTONE_ASSERT(width > 0);
	BSTONE_ASSERT(height > 0);
	BSTONE_ASSERT(sample_count >= 0);
	BSTONE_ASSERT(gl_internal_format > 0);

	auto rbo_resource = create_renderbuffer();
	bind_renderbuffer(rbo_resource.get());

	BSTONE_ASSERT(gl_device_features_.is_framebuffer_available);

	if (sample_count > 1)
	{
		glRenderbufferStorageMultisample_(GL_RENDERBUFFER, sample_count, gl_internal_format, width, height);
		GlR3rError::check_optionally();
	}
	else
	{
		glRenderbufferStorage_(GL_RENDERBUFFER, gl_internal_format, width, height);
		GlR3rError::check_optionally();
	}

	bind_renderbuffer(0);
	return rbo_resource;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::destroy_msaa_color_rb() noexcept
{
	msaa_color_rb_.reset();
}

void GlR3rImpl::destroy_msaa_depth_rb() noexcept
{
	msaa_depth_rb_.reset();
}

void GlR3rImpl::destroy_msaa_fbo() noexcept
{
	msaa_fbo_.reset();
}

void GlR3rImpl::destroy_msaa_framebuffer() noexcept
{
	destroy_msaa_fbo();
	destroy_msaa_color_rb();
	destroy_msaa_depth_rb();
}

void GlR3rImpl::create_msaa_color_rb(int width, int height, int sample_count)
try {
	msaa_color_rb_ = create_renderbuffer(width, height, sample_count, GL_RGBA8);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::create_msaa_depth_rb(int width, int height, int sample_count)
try {
	msaa_depth_rb_ = create_renderbuffer(width, height, sample_count, GL_DEPTH_COMPONENT);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::create_msaa_framebuffer()
try {
	if (aa_type_ == R3rAaType::none || device_features_.max_msaa_degree < R3rLimits::min_aa_on())
	{
		return;
	}

	const auto aa_degree = clamp(aa_value_, R3rLimits::min_aa_on(), device_features_.max_msaa_degree);

	create_msaa_color_rb(screen_width_, screen_height_, aa_degree);
	create_msaa_depth_rb(screen_width_, screen_height_, aa_degree);

	msaa_fbo_ = create_framebuffer();
	bind_framebuffer(GL_FRAMEBUFFER, msaa_fbo_.get());

	glFramebufferRenderbuffer_(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_RENDERBUFFER,
		msaa_color_rb_.get());

	glFramebufferRenderbuffer_(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER,
		msaa_depth_rb_.get());

	const auto framebuffer_status = glCheckFramebufferStatus_(GL_FRAMEBUFFER);

	if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE)
	{
		BSTONE_THROW_STATIC_SOURCE("Incomplete framebuffer object.");
	}

	bind_framebuffer(GL_FRAMEBUFFER, 0);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::destroy_framebuffers() noexcept
{
	destroy_msaa_framebuffer();
}

void GlR3rImpl::initialize_framebuffer_funcs() noexcept
{
	if (gl_device_features_.is_framebuffer_ext)
	{
		glBindFramebuffer_ = glBindFramebufferEXT;
		glBindRenderbuffer_ = glBindRenderbufferEXT;
		glBlitFramebuffer_ = glBlitFramebufferEXT;
		glCheckFramebufferStatus_ = glCheckFramebufferStatusEXT;
		glDeleteFramebuffers_ = glDeleteFramebuffersEXT;
		glDeleteRenderbuffers_ = glDeleteRenderbuffersEXT;
		glFramebufferRenderbuffer_ = glFramebufferRenderbufferEXT;
		glGenFramebuffers_ = glGenFramebuffersEXT;
		glGenRenderbuffers_ = glGenRenderbuffersEXT;
		glRenderbufferStorage_ = glRenderbufferStorageEXT;
		glRenderbufferStorageMultisample_ = glRenderbufferStorageMultisampleEXT;
	}
	else
	{
		glBindFramebuffer_ = glBindFramebuffer;
		glBindRenderbuffer_ = glBindRenderbuffer;
		glBlitFramebuffer_ = glBlitFramebuffer;
		glCheckFramebufferStatus_ = glCheckFramebufferStatus;
		glDeleteFramebuffers_ = glDeleteFramebuffers;
		glDeleteRenderbuffers_ = glDeleteRenderbuffers;
		glFramebufferRenderbuffer_ = glFramebufferRenderbuffer;
		glGenFramebuffers_ = glGenFramebuffers;
		glGenRenderbuffers_ = glGenRenderbuffers;
		glRenderbufferStorage_ = glRenderbufferStorage;
		glRenderbufferStorageMultisample_ = glRenderbufferStorageMultisample;
	}
}

void GlR3rImpl::create_framebuffers()
try {
	if (!gl_device_features_.is_framebuffer_available)
	{
		return;
	}

	initialize_framebuffer_funcs();
	create_msaa_framebuffer();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::blit_framebuffers()
try {
	if (msaa_fbo_.get() == 0)
	{
		return;
	}

	// MSAA FBO -> Default FBO
	//

	// Read: MSAA
	// Draw: Default
	bind_framebuffer(GL_DRAW_FRAMEBUFFER, 0);

	blit_framebuffer(
		screen_width_,
		screen_height_,
		screen_width_,
		screen_height_,
		false);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::bind_framebuffers()
try {
	if (msaa_fbo_.get() == 0)
	{
		return;
	}

	bind_framebuffer(GL_FRAMEBUFFER, msaa_fbo_.get());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::bind_framebuffers_for_read_pixels()
try {
	if (msaa_fbo_.get() == 0)
	{
		return;
	}

	bind_framebuffer(GL_FRAMEBUFFER, 0);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::disable_aa()
try {
	aa_type_ = R3rAaType::none;

	if (msaa_fbo_.get() == 0)
	{
		return;
	}

	destroy_msaa_framebuffer();
	create_msaa_framebuffer();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::set_msaa(int aa_value)
try {
	if (device_features_.is_msaa_requires_restart)
	{
		BSTONE_THROW_STATIC_SOURCE("Requires restart.");
	}

	if (!gl_device_features_.is_framebuffer_available)
	{
		BSTONE_THROW_STATIC_SOURCE("Framebuffer not available.");
	}

	if (aa_type_ == R3rAaType::ms && aa_value_ == aa_value)
	{
		return;
	}

	aa_type_ = R3rAaType::ms;
	aa_value_ = aa_value;

	destroy_framebuffers();
	create_framebuffers();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_clear(const R3rClearCmd& command)
try {
	context_->clear(command.clear.color);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_culling(const R3rEnableCullingCmd& command)
try {
	context_->enable_culling(command.is_enable);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_enable_depth_test(const R3rEnableDepthTestCmd& command)
try {
	context_->enable_depth_test(command.is_enable);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_enable_depth_write(const R3rEnableDepthWriteCmd& command)
try {
	context_->enable_depth_write(command.is_enable);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_viewport(const R3rSetViewportCmd& command)
try {
	context_->set_viewport(command.viewport);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_enable_blending(const R3rEnableBlendingCmd& command)
try {
	context_->enable_blending(command.is_enable);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_blending_func(const R3rSetBlendingFuncCmd& command)
try {
	context_->set_blending_func(command.blending_func);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_enable_scissor(const R3rEnableScissorCmd& command)
try {
	context_->enable_scissor(command.is_enable);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_scissor_box(const R3rSetScissorBoxCmd& command)
try {
	context_->set_scissor_box(command.scissor_box);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_texture(const R3rSetTextureCmd& command)
try {
	context_->set_r2_texture(static_cast<GlR3rR2Texture*>(command.r2_texture));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_sampler(const R3rSetSamplerCmd& command)
try {
	context_->set_sampler(static_cast<GlR3rSampler*>(command.sampler));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_vertex_input(const R3rSetVertexInputCmd& command)
try {
	context_->set_vertex_input(static_cast<GlR3rVertexInput*>(command.vertex_input));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_shader_stage(const R3rSetShaderStageCmd& command)
try {
	context_->set_shader_stage(static_cast<GlR3rShaderStage*>(command.shader_stage));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_int32_uniform(const R3rSetInt32UniformCmd& command)
try {
	if (command.var == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null variable.");
	}

	command.var->set_int32(command.value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_float32_uniform(const R3rSetFloat32UniformCmd& command)
try {
	if (command.var == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null variable.");
	}

	command.var->set_float32(command.value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_vec2_uniform(const R3rSetVec2UniformCmd& command)
try {
	if (command.var == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null variable.");
	}

	command.var->set_vec2(command.value.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_vec4_uniform(const R3rSetVec4UniformCmd& command)
try {
	if (command.var == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null variable.");
	}

	command.var->set_vec4(command.value.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_mat4_uniform(const R3rSetMat4UniformCmd& command)
try {
	if (command.var == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null variable.");
	}

	command.var->set_mat4(command.value.data());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_set_sampler_2d_uniform(const R3rSetR2SamplerUniformCmd& command)
try {
	if (command.var == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null variable.");
	}

	command.var->set_r2_sampler(command.value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rImpl::submit_draw_indexed(const R3rDrawIndexedCmd& command)
try {
	const auto& param = command.draw_indexed;

	auto gl_primitive_topology = GLenum{};

	switch (param.primitive_type)
	{
		case R3rPrimitiveType::point_list:
			gl_primitive_topology = GL_POINTS;
			break;

		case R3rPrimitiveType::line_list:
			gl_primitive_topology = GL_LINES;
			break;

		case R3rPrimitiveType::line_strip:
			gl_primitive_topology = GL_LINE_STRIP;
			break;

		case R3rPrimitiveType::triangle_list:
			gl_primitive_topology = GL_TRIANGLES;
			break;

		case R3rPrimitiveType::triangle_strip:
			gl_primitive_topology = GL_TRIANGLE_STRIP;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported primitive topology.");
	}

	if (param.vertex_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Vertex count out of range.");
	}

	if (param.vertex_count == 0)
	{
		return;
	}

	switch (param.index_byte_depth)
	{
		case 1:
		case 2:
		case 4:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported index value byte depth.");
	}

	if (param.index_buffer_offset < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Offset to indices out of range.");
	}

	if (param.index_offset < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Index offset out of range.");
	}

	// Vertex input.
	//
	const auto vertex_input = context_->get_vertex_input();

	if (vertex_input == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null current vertex input.");
	}

	context_->get_vertex_input_manager().set(*vertex_input);

	// Shader stage.
	//
	context_->bind_shader_stage(context_->get_shader_stage());

	// Sampler.
	//
	context_->get_sampler_manager().set(context_->get_sampler());

	// Textures.
	//
	context_->bind_r2_texture(context_->get_r2_texture());

	// Index buffer.
	//
	auto index_buffer = static_cast<GlR3rBuffer*>(vertex_input->get_index_buffer());

	if (index_buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null index buffer.");
	}

	const auto index_buffer_offset = param.index_buffer_offset + (param.index_offset * param.index_byte_depth);

	const auto index_buffer_indices = reinterpret_cast<const void*>(
		static_cast<std::intptr_t>(index_buffer_offset));

	const auto gl_element_type = GlR3rUtils::index_buffer_get_element_type_by_byte_depth(
		param.index_byte_depth);

	index_buffer->set(true);

	// Draw it.
	//
	glDrawElements(
		gl_primitive_topology, // mode
		param.vertex_count, // count
		gl_element_type, // type
		index_buffer_indices // indices
	);

	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ==========================================================================

R3rUPtr make_gl_r3r(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param)
try {
	return std::make_unique<GlR3rImpl>(video_mgr, window_mgr, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
