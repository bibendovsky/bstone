/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer

#include "bstone_null_r3r.h"
#include <stddef.h>
#include <stdint.h>
#include "bstone_assert.h"
#include "bstone_single_pool_resource.h"
#include "bstone_utility.h"
#include "bstone_r3r_limits.h"
#include "bstone_null_r3r_buffer.h"
#include "bstone_null_r3r_r2_texture.h"
#include "bstone_null_r3r_sampler.h"
#include "bstone_null_r3r_shader.h"
#include "bstone_null_r3r_shader_stage.h"
#include "bstone_null_r3r_vertex_input.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rImpl final : public R3r
{
public:
	NullR3rImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param);
	~NullR3rImpl() override;

	void* operator new(size_t size);
	void operator delete(void* ptr);

private:
	R3rType do_get_type() const noexcept override;
	std::string_view do_get_name() const noexcept override;
	std::string_view do_get_description() const noexcept override;

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
	void do_wait_for_device() override;

private:
	using MemoryPool = SinglePoolResource<NullR3rImpl>;

private:
	static MemoryPool memory_pool_;

private:
	sys::VideoMgr& video_mgr_;
	sys::WindowMgr& window_mgr_;

	R3rType type_{};
	std::string_view name_{};
	std::string_view description_{};
	R3rDeviceFeatures r3r_device_features_{};
	R3rDeviceInfo device_info_{};
	sys::WindowUPtr window_{};

private:
	void initialize_device_features();
	void initialize_window();
};

// --------------------------------------------------------------------------

NullR3rImpl::MemoryPool NullR3rImpl::memory_pool_{};

// --------------------------------------------------------------------------

NullR3rImpl::~NullR3rImpl() = default;

NullR3rImpl::NullR3rImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param)
try
	:
	video_mgr_{video_mgr},
	window_mgr_{window_mgr},
	type_{param.renderer_type},
	name_{"NULL"},
	description_{"Dummy"}
{
	BSTONE_ASSERT(param.renderer_type == R3rType::null);

	initialize_device_features();
	initialize_window();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void* NullR3rImpl::operator new(size_t size)
try {
	return memory_pool_.allocate(static_cast<intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullR3rImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

R3rType NullR3rImpl::do_get_type() const noexcept
{
	return type_;
}

std::string_view NullR3rImpl::do_get_name() const noexcept
{
	return name_;
}

std::string_view NullR3rImpl::do_get_description() const noexcept
{
	return description_;
}

const R3rDeviceFeatures& NullR3rImpl::do_get_device_features() const noexcept
{
	return r3r_device_features_;
}

const R3rDeviceInfo& NullR3rImpl::do_get_device_info() const noexcept
{
	return device_info_;
}

void NullR3rImpl::do_enable_checking_api_calls_for_errors(bool is_enable)
{
	maybe_unused(is_enable);
}

sys::Window& NullR3rImpl::do_get_window() const noexcept
{
	return *window_;
}

void NullR3rImpl::do_handle_resize(sys::WindowSize new_size)
{
	maybe_unused(new_size);
}

bool NullR3rImpl::do_get_vsync() const noexcept
{
	return false;
}

void NullR3rImpl::do_enable_vsync(bool is_enabled)
{
	maybe_unused(is_enabled);
}

void NullR3rImpl::do_set_anti_aliasing(R3rAaType aa_type, int aa_value)
{
	maybe_unused(aa_type);
	maybe_unused(aa_value);
}

void NullR3rImpl::do_read_pixels(
	sys::PixelFormat pixel_format,
	void* buffer,
	bool& is_flipped_vertically)
{
	maybe_unused(pixel_format);
	maybe_unused(buffer);
	maybe_unused(is_flipped_vertically);
}

void NullR3rImpl::do_present()
{}

R3rBufferUPtr NullR3rImpl::do_create_buffer(const R3rBufferInitParam& param)
try {
	return make_null_r3r_buffer(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rVertexInputUPtr NullR3rImpl::do_create_vertex_input(const R3rCreateVertexInputParam& param)
try {
	return make_null_r3r_vertex_input(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderUPtr NullR3rImpl::do_create_shader(const R3rShaderInitParam& param)
try {
	return make_null_r3r_shader(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderStageUPtr NullR3rImpl::do_create_shader_stage(const R3rShaderStageInitParam& param)
try {
	return make_null_r3r_shader_stage(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rR2TextureUPtr NullR3rImpl::do_create_r2_texture(const R3rR2TextureInitParam& param)
try {
	return make_null_r3r_r2_texture(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSamplerUPtr NullR3rImpl::do_create_sampler(const R3rSamplerInitParam& param)
try {
	return make_null_r3r_sampler(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullR3rImpl::do_submit_commands(Span<R3rCmdBuffer*> command_buffers)
{
	maybe_unused(command_buffers);
}

void NullR3rImpl::do_wait_for_device()
{}

void NullR3rImpl::initialize_device_features()
{
	r3r_device_features_.max_texture_dimension = 8192;
	r3r_device_features_.max_viewport_width = 8192;
	r3r_device_features_.max_viewport_height = 8192;
	r3r_device_features_.is_anisotropy_available = false;
	r3r_device_features_.is_npot_available = true;
	r3r_device_features_.is_mipmap_available = true;
	r3r_device_features_.is_sampler_available = true;
	r3r_device_features_.max_vertex_input_locations = 16;
}

void NullR3rImpl::initialize_window()
{
	sys::WindowInitParam param{};
	param.x = sys::WindowOffset::make_centered();
	param.y = sys::WindowOffset::make_centered();
	param.width = R3rLimits::min_viewport_width();
	param.height = R3rLimits::min_viewport_height();
	param.rounded_corner_type = sys::WindowRoundedCornerType::none;
	param.fullscreen_type = sys::WindowFullscreenType::none;
	param.renderer_type = sys::WindowRendererType::none;

	window_ = window_mgr_.make_window(param);
}

} // namespace

// ==========================================================================

R3rUPtr make_null_r3r(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param)
try {
	return std::make_unique<NullR3rImpl>(video_mgr, window_mgr, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
