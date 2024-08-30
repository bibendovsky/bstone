/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer

#include "bstone_exception.h"
#include "bstone_r3r.h"

namespace bstone {

R3r::R3r() noexcept = default;

R3r::~R3r() = default;

R3rType R3r::get_type() const noexcept
{
	return do_get_type();
}

StringView R3r::get_name() const noexcept
{
	return do_get_name();
}

StringView R3r::get_description() const noexcept
{
	return do_get_description();
}

const R3rDeviceFeatures& R3r::get_device_features() const noexcept
{
	return do_get_device_features();
}

const R3rDeviceInfo& R3r::get_device_info() const noexcept
{
	return do_get_device_info();
}

void R3r::enable_checking_api_calls_for_errors(bool is_enable)
{
	do_enable_checking_api_calls_for_errors(is_enable);
}

sys::Window& R3r::get_window() const noexcept
{
	return do_get_window();
}

void R3r::handle_resize(sys::WindowSize new_size)
try {
	return do_handle_resize(new_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool R3r::get_vsync() const noexcept
{
	return do_get_vsync();
}

void R3r::enable_vsync(bool is_enabled)
try {
	do_enable_vsync(is_enabled);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3r::set_anti_aliasing(R3rAaType aa_type, int aa_value)
try {
	do_set_anti_aliasing(aa_type, aa_value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3r::read_pixels(
	sys::PixelFormat pixel_format,
	void* buffer,
	bool& is_flipped_vertically)
try {
	do_read_pixels(pixel_format, buffer, is_flipped_vertically);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3r::present()
try {
	do_present();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rBufferUPtr R3r::create_buffer(const R3rBufferInitParam& param)
try {
	return do_create_buffer(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rR2TextureUPtr R3r::create_r2_texture(const R3rR2TextureInitParam& param)
try {
	return do_create_r2_texture(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSamplerUPtr R3r::create_sampler(const R3rSamplerInitParam& param)
try {
	return do_create_sampler(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rVertexInputUPtr R3r::create_vertex_input(const R3rCreateVertexInputParam& param)
try {
	return do_create_vertex_input(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderUPtr R3r::create_shader(const R3rShaderInitParam& param)
try {
	return do_create_shader(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderStageUPtr R3r::create_shader_stage(const R3rShaderStageInitParam& param)
try {
	return do_create_shader_stage(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3r::submit_commands(Span<R3rCmdBuffer*> command_buffers)
try {
	return do_submit_commands(command_buffers);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
