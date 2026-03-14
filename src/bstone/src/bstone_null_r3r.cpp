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
	~NullR3rImpl() override = default;

	R3rType get_type() const override;
	std::string_view get_name() const override;
	std::string_view get_description() const override;

	const R3rDeviceFeatures& get_device_features() const override;
	const R3rDeviceInfo& get_device_info() const override;

	void enable_checking_api_calls_for_errors(bool is_enable) override;

	sys::Window& get_window() const override;
	void handle_resize(sys::WindowSize new_size) override;

	bool get_vsync() const override;
	void enable_vsync(bool is_enabled) override;

	void set_anti_aliasing(R3rAaType aa_type, int aa_value) override;

	void read_pixels(
		sys::PixelFormat pixel_format,
		void* buffer,
		bool& is_flipped_vertically) override;

	void present() override;

	R3rBufferUPtr create_buffer(const R3rBufferInitParam& param) override;
	R3rR2TextureUPtr create_r2_texture(const R3rR2TextureInitParam& param) override;
	R3rSamplerUPtr create_sampler(const R3rSamplerInitParam& param) override;
	R3rVertexInputUPtr create_vertex_input(const R3rCreateVertexInputParam& param) override;
	R3rShaderUPtr create_shader(const R3rShaderInitParam& param) override;
	R3rShaderStageUPtr create_shader_stage(const R3rShaderStageInitParam& param) override;
	void submit_commands(std::span<R3rCmdBuffer*> command_buffers) override;
	void wait_for_device() override;

private:
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

NullR3rImpl::NullR3rImpl([[maybe_unused]] sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param)
try
	:
	window_mgr_{window_mgr},
	type_{param.renderer_type},
	name_{"NULL"},
	description_{"Dummy"}
{
	BSTONE_ASSERT(param.renderer_type == R3rType::null);

	initialize_device_features();
	initialize_window();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rType NullR3rImpl::get_type() const
{
	return type_;
}

std::string_view NullR3rImpl::get_name() const
{
	return name_;
}

std::string_view NullR3rImpl::get_description() const
{
	return description_;
}

const R3rDeviceFeatures& NullR3rImpl::get_device_features() const
{
	return r3r_device_features_;
}

const R3rDeviceInfo& NullR3rImpl::get_device_info() const
{
	return device_info_;
}

void NullR3rImpl::enable_checking_api_calls_for_errors([[maybe_unused]] bool is_enable)
{}

sys::Window& NullR3rImpl::get_window() const
{
	return *window_;
}

void NullR3rImpl::handle_resize([[maybe_unused]] sys::WindowSize new_size)
{}

bool NullR3rImpl::get_vsync() const
{
	return false;
}

void NullR3rImpl::enable_vsync([[maybe_unused]] bool is_enabled)
{}

void NullR3rImpl::set_anti_aliasing([[maybe_unused]] R3rAaType aa_type, [[maybe_unused]] int aa_value)
{}

void NullR3rImpl::read_pixels(
	[[maybe_unused]] sys::PixelFormat pixel_format,
	[[maybe_unused]] void* buffer,
	[[maybe_unused]] bool& is_flipped_vertically)
{}

void NullR3rImpl::present()
{}

R3rBufferUPtr NullR3rImpl::create_buffer(const R3rBufferInitParam& param)
try {
	return make_null_r3r_buffer(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rVertexInputUPtr NullR3rImpl::create_vertex_input(const R3rCreateVertexInputParam& param)
try {
	return make_null_r3r_vertex_input(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderUPtr NullR3rImpl::create_shader(const R3rShaderInitParam& param)
try {
	return make_null_r3r_shader(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderStageUPtr NullR3rImpl::create_shader_stage(const R3rShaderStageInitParam& param)
try {
	return make_null_r3r_shader_stage(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rR2TextureUPtr NullR3rImpl::create_r2_texture(const R3rR2TextureInitParam& param)
try {
	return make_null_r3r_r2_texture(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSamplerUPtr NullR3rImpl::create_sampler(const R3rSamplerInitParam& param)
try {
	return make_null_r3r_sampler(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullR3rImpl::submit_commands([[maybe_unused]] std::span<R3rCmdBuffer*> command_buffers)
{}

void NullR3rImpl::wait_for_device()
{}

void NullR3rImpl::initialize_device_features()
{
	r3r_device_features_.max_texture_dimension = 8192;
	r3r_device_features_.max_viewport_width = 8192;
	r3r_device_features_.max_viewport_height = 8192;
	r3r_device_features_.is_anisotropy_available = false;
	r3r_device_features_.is_npot_available = true;
	r3r_device_features_.can_generate_mipmap = true;
	r3r_device_features_.is_sampler_available = true;
	r3r_device_features_.max_vertex_input_locations = 16;
}

void NullR3rImpl::initialize_window()
{
	sys::WindowInitParam param{};
	param.x = sys::WindowOffset::make_centered();
	param.y = sys::WindowOffset::make_centered();
	param.width = R3rLimits::min_viewport_width;
	param.height = R3rLimits::min_viewport_height;
	param.rounded_corner_type = sys::WindowRoundedCornerType::none;
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
