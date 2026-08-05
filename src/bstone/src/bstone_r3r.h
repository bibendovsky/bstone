/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer

#ifndef BSTONE_R3R_INCLUDED
#define BSTONE_R3R_INCLUDED

#include <memory>
#include <span>
#include <string_view>

#include "bstone_exception.h"

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

struct R3rReadPixelsParam
{
	// Pixel format of the destination.
	sys::PixelFormat pixel_format;

	// Width of the destination in pixels.
	int width;

	// Height of the destination in pixels.
	int height;

	// Destination buffer with tightly packed rows.
	void* buffer;
};

// Throws unless the destination describes an image of exactly the specified
// size. A renderer packs the rows on its own, so it can neither crop nor
// re-pack them to fit a destination of some other size.
inline void r3r_validate_read_pixels_param(
	const R3rReadPixelsParam& param,
	int width,
	int height)
{
	if (param.pixel_format != sys::PixelFormat::r8g8b8)
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported pixel format.");
	}

	if (param.buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null destination buffer.");
	}

	if (param.width != width || param.height != height)
	{
		BSTONE_THROW_STATIC_SOURCE("Destination size mismatch.");
	}
}

// ==========================================================================

class R3r
{
public:
	R3r() = default;
	virtual ~R3r() = default;

	virtual R3rType get_type() const = 0;
	virtual std::string_view get_name() const = 0;
	virtual std::string_view get_description() const = 0;

	virtual const R3rDeviceFeatures& get_device_features() const = 0;
	virtual const R3rDeviceInfo& get_device_info() const = 0;

	virtual void enable_checking_api_calls_for_errors(bool is_enable) = 0;

	virtual sys::Window& get_window() const = 0;
	virtual void handle_resize(sys::WindowSize new_size) = 0;

	// Size of the rendered image in pixels.
	virtual sys::WindowSize get_screen_size() const = 0;

	virtual bool get_vsync() const = 0;
	virtual void enable_vsync(bool is_enabled) = 0;

	virtual void set_anti_aliasing(R3rAaType aa_type, int aa_value) = 0;

	virtual void read_pixels(
		const R3rReadPixelsParam& param,
		bool& is_flipped_vertically) = 0;

	virtual void present() = 0;

	virtual R3rBufferUPtr create_buffer(const R3rBufferInitParam& param) = 0;
	virtual R3rR2TextureUPtr create_r2_texture(const R3rR2TextureInitParam& param) = 0;
	virtual R3rSamplerUPtr create_sampler(const R3rSamplerInitParam& param) = 0;
	virtual R3rVertexInputUPtr create_vertex_input(const R3rCreateVertexInputParam& param) = 0;
	virtual R3rShaderUPtr create_shader(const R3rShaderInitParam& param) = 0;
	virtual R3rShaderStageUPtr create_shader_stage(const R3rShaderStageInitParam& param) = 0;

	virtual void submit_commands(std::span<R3rCmdBuffer*> command_buffers) = 0;
	virtual void wait_for_device() = 0;
};

// ==========================================================================

using R3rUPtr = std::unique_ptr<R3r>;

} // namespace bstone

#endif // BSTONE_R3R_INCLUDED
