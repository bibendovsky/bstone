/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer

#ifndef BSTONE_R3R_INCLUDED
#define BSTONE_R3R_INCLUDED

#include <memory>

#include "bstone_span.h"
#include "bstone_string_view.h"

#include "bstone_sys_pixel_format.h"
#include "bstone_sys_window.h"

#include "bstone_r3r_buffer.h"
#include "bstone_r3r_r2_texture.h"
#include "bstone_r3r_sampler.h"
#include "bstone_r3r_shader.h"
#include "bstone_r3r_shader_stage.h"
#include "bstone_r3r_types.h"
#include "bstone_r3r_vertex_input.h"

namespace bstone {

class R3rCmdBuffer;

// ==========================================================================

struct R3rInitParam
{
	R3rType renderer_type;

	R3rAaType aa_type;
	int aa_value;

	bool is_vsync;
};

struct R3rDrawIndexedParam
{
	// Primitive topology.
	R3rPrimitiveType primitive_type;

	// Vertex count.
	int vertex_count;

	// Size of the element in bytes.
	int index_byte_depth;

	// Offset in bytes from the start of index buffer.
	int index_buffer_offset;

	// Number of the index to draw from.
	int index_offset;
};

// ==========================================================================

class R3r
{
public:
	R3r() noexcept;
	virtual ~R3r();

	R3rType get_type() const noexcept;
	StringView get_name() const noexcept;
	StringView get_description() const noexcept;

	const R3rDeviceFeatures& get_device_features() const noexcept;
	const R3rDeviceInfo& get_device_info() const noexcept;

	void enable_checking_api_calls_for_errors(bool is_enable);

	sys::Window& get_window() const noexcept;
	void handle_resize(sys::WindowSize new_size);

	bool get_vsync() const noexcept;
	void enable_vsync(bool is_enabled);

	void set_anti_aliasing(R3rAaType aa_type, int aa_value);

	void read_pixels(sys::PixelFormat pixel_format, void* buffer, bool& is_flipped_vertically);

	void present();

	R3rBufferUPtr create_buffer(const R3rBufferInitParam& param);
	R3rR2TextureUPtr create_r2_texture(const R3rR2TextureInitParam& param);
	R3rSamplerUPtr create_sampler(const R3rSamplerInitParam& param);
	R3rVertexInputUPtr create_vertex_input(const R3rCreateVertexInputParam& param);
	R3rShaderUPtr create_shader(const R3rShaderInitParam& param);
	R3rShaderStageUPtr create_shader_stage(const R3rShaderStageInitParam& param);

	void submit_commands(Span<R3rCmdBuffer*> command_buffers);

private:
	virtual R3rType do_get_type() const noexcept = 0;
	virtual StringView do_get_name() const noexcept = 0;
	virtual StringView do_get_description() const noexcept = 0;

	virtual const R3rDeviceFeatures& do_get_device_features() const noexcept = 0;
	virtual const R3rDeviceInfo& do_get_device_info() const noexcept = 0;

	virtual void do_enable_checking_api_calls_for_errors(bool is_enable) = 0;

	virtual sys::Window& do_get_window() const noexcept = 0;
	virtual void do_handle_resize(sys::WindowSize new_size) = 0;

	virtual bool do_get_vsync() const noexcept = 0;
	virtual void do_enable_vsync(bool is_enabled) = 0;

	virtual void do_set_anti_aliasing(R3rAaType aa_type, int aa_value) = 0;

	virtual void do_read_pixels(
		sys::PixelFormat pixel_format,
		void* buffer,
		bool& is_flipped_vertically) = 0;

	virtual void do_present() = 0;

	virtual R3rBufferUPtr do_create_buffer(const R3rBufferInitParam& param) = 0;
	virtual R3rR2TextureUPtr do_create_r2_texture(const R3rR2TextureInitParam& param) = 0;
	virtual R3rSamplerUPtr do_create_sampler(const R3rSamplerInitParam& param) = 0;
	virtual R3rVertexInputUPtr do_create_vertex_input(const R3rCreateVertexInputParam& param) = 0;
	virtual R3rShaderUPtr do_create_shader(const R3rShaderInitParam& param) = 0;
	virtual R3rShaderStageUPtr do_create_shader_stage(const R3rShaderStageInitParam& param) = 0;

	virtual void do_submit_commands(Span<R3rCmdBuffer*> command_buffers) = 0;
};

// ==========================================================================

using R3rUPtr = std::unique_ptr<R3r>;

} // namespace bstone

#endif // BSTONE_R3R_INCLUDED
